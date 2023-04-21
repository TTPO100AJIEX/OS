#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "log/log.h"
#include "utils/utils.h"
#include "hotel/hotel.h"
#include "visitor/visitor.h"

static bool running = true;
struct State state;
pid_t hotelPID;
static void stop()
{
    if (!running) return;
    running = false;
    fclose(stdin);

    printf("\nStopping...\n");
    if (kill(hotelPID, SIGINT) == -1) perror("Controller: failed to stop the hotel");
    waitpid(hotelPID, NULL, 0);

    if (clear_state(state) == -1)
    {
        perror("Controller: failed to clear the state");
        exit(-1);
    }

    exit(0);
}

int main(int argc, char** argv)
{
    setbuf(stdout, NULL); // Remove buffering of stdout
    signal(SIGINT, stop);
    if (argc < 2) { printf("Not enough command line arguments specified!\n"); return 1; }
    state = init_state(argv[1]);
    if (state.door_in == NULL || state.door_out == NULL || state.reception_in == NULL || state.reception_out == NULL || state.shared_memory == NULL)
    {
        perror("Controller: failed to initialize the state");
        clear_state(state);
        return 1;
    }


    pid_t hotelPID = fork();
    if (hotelPID == -1) { perror("Controller: failed to create the hotel"); return 1; }
    if (hotelPID == 0) return hotel(state);
    log("Controller: started hotel with PID %d\n", hotelPID);


    bool showMessage = true;
    while (running)
    {
        if (showMessage) printf("Please enter the gender ('m'/'f') of the next visitor: ");
        char gender = getchar();
        showMessage = false;
        if (gender <= ' ') continue;
        showMessage = true;
        if (gender != 'm' && gender != 'f') { printf("Gender must be 'm' or 'f'\n"); continue; }

        printf("Please enter the time (in seconds between 1 and 15) the visitor needs a room for: ");
        unsigned int time;
        if (scanf("%u", &time) == EOF) break;
        if (time < 1 || time > 15) { printf("Time must be between 1 and 15\n"); continue; }
        
        pid_t visitorPID = fork();
        if (visitorPID == -1) { perror("Controller: failed to create a visitor"); continue; }
        if (visitorPID == 0) return visitor(state, gender == 'm' ? MALE : FEMALE, time);
        log("Controller: started %c visitor with PID %d (time: %u)\n", gender, visitorPID, time);
    }
}