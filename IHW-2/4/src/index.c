#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include "protocol.h"
#include "log/log.h"
#include "hotel/hotel.h"
#include "visitor/visitor.h"

static bool running = true;
static void stop()
{
    running = false;
    fclose(stdin);
}

int main(char argc, char** argv)
{
    setbuf(stdout, NULL); // Remove buffering of stdout
    signal(SIGINT, stop);
    if (set_log_file(argv[1]) == -1) { perror("Controller: failed to setup the logging"); return 1; }
    
    int shm_id = shm_open(memory, O_CREAT | O_RDWR, 0666);
    if (shm_id == -1) { perror("Controller: failed to create a shared memory"); return 1; }
    if (ftruncate(shm_id, sizeof(struct Message)) == -1) { perror("Controller: failed to size the shared memory"); return 1; }
    struct State state = {
        .door = sem_open(door_semaphore, O_CREAT, 0666, 0),
        .reception_in = sem_open(reception_in_semaphore, O_CREAT, 0666, 0),
        .reception_out = sem_open(reception_out_semaphore, O_CREAT, 0666, 0),
        .shared_memory = mmap(0, sizeof(struct Message), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0)
    };
    if (state.door == SEM_FAILED || state.reception_in == SEM_FAILED || state.reception_out == SEM_FAILED) { perror("Controller: failed to create the semaphores"); return 1; }
    if (state.shared_memory == MAP_FAILED) { perror("Controller: failed to load the shared memory"); return 1; }
    

    pid_t hotelPID = fork();
    if (hotelPID == -1) { perror("Controller: failed to create the hotel"); return 1; }
    if (hotelPID == 0) return hotel(state);
    log("Controller: started hotel with PID %d\n", hotelPID);

    bool showMessage = true;
    while (running)
    {
        if (showMessage) printf("Please enter the gender (m/f) of the next visitor: ");
        char gender = getchar();
        showMessage = false;
        if (gender <= ' ') continue;
        showMessage = true;
        if (gender != 'm' && gender != 'f')
        {
            printf("Gender must be 'm' or 'f'\n");
            continue;
        }

        printf("Please enter the time (in seconds between 1 and 15) the visitor needs a room for: ");
        unsigned int time;
        if (scanf("%u", &time) == EOF) break;
        if (time < 1 || time > 15)
        {
            printf("Time must be between 1 and 15\n");
            continue;
        }
        
        pid_t visitorPID = fork();
        if (visitorPID == -1) perror("Controller: failed to craete a visitor");
        if (visitorPID == 0) return visitor(state, gender == 'm' ? MALE : FEMALE, time);
        log("Controller: started %c visitor with PID %d (time: %u)\n", gender, visitorPID, time);
    }


    printf("\n\nStopping...\n");
    if (kill(hotelPID, SIGINT) == -1) perror("Controller: failed to stop the hotel");
    waitpid(hotelPID, NULL, 0);

    if (shm_unlink(memory) == -1) perror("Controller: failed to delete the shared memory");

    if (sem_close(state.door) == -1) perror("Controller: failed to close the door semaphore");
    if (sem_unlink(door_semaphore) == -1) perror("Controller: failed to delete the door semaphore");
    
    if (sem_close(state.reception_in) == -1) perror("Controller: failed to close the reception_in semaphore");
    if (sem_unlink(reception_in_semaphore) == -1) perror("Controller: failed to delete the reception_in semaphore");
    
    if (sem_close(state.reception_out) == -1) perror("Controller: failed to close the reception_out semaphore");
    if (sem_unlink(reception_out_semaphore) == -1) perror("Controller: failed to delete the reception_out semaphore");

    if (stop_logging() == -1) perror("Controller: failed to stop the logging");
    return 0;
}