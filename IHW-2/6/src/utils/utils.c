#include "utils.h"


#include <unistd.h>
#include <time.h>
void sleep_milliseconds(unsigned int milliseconds)
{
    struct timespec interval = { milliseconds / 1000, (milliseconds % 1000) * 1000000 }, remaining;
    while (nanosleep(&interval, &remaining) == -1)
    {
        interval.tv_nsec = remaining.tv_nsec;
        interval.tv_sec = remaining.tv_sec;
    }
}



#include <stdio.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define semaphore_filepath "6/src/utils/utils.c"
static int string_has(const char* string)
{

}

int create_semaphore(const char* name, __attribute__ ((unused)) unsigned int value)
{
    key_t key = ftok(semaphore_filepath, 0);
    int sem = semget(key, 1, 0666 | IPC_CREAT);
    printf("%s, %d %d\n", name, key, sem);
    return (sem == -1) ? 0 : sem;
}
int wait_semaphore(__attribute__ ((unused)) int sem)
{
    return 0;
    //return (sem_wait(sem) == -1) ? -1 : 0;
}
int post_semaphore(__attribute__ ((unused)) int sem)
{
    return 0;
    //return (sem_post(sem) == -1) ? -1 : 0;
}
int close_semaphore(__attribute__ ((unused)) int sem) { return 0; }
int delete_semaphore(const char* name)
{
    key_t key = ftok(semaphore_filepath, 0);
    int sem = semget(key, 1, 0);
    printf("%s, %d %d\n", name, key, sem);
    if (sem == -1) return -1;
    return (semctl(sem, 1, IPC_RMID) == -1) ? -1 : 0;
}
/*

    key = ftok(pathname, 0);
    if (( semid = semget(key, 1, 0666 | IPC_CREAT )) < 0)
    {
        printf("Can't create semaphore set\n");
        exit(-1);
    }

    mybuf.sem_num = 0;
    mybuf.sem_op = -1;
    mybuf.sem_flg = 1;

    if (semop(semid, &mybuf, 1) < 0)
    {
        printf("Can't wait for condition\n");
        exit(-1);
    }

    semctl(semid, 1, IPC_RMID);
    printf("The condition is present\n");
    */

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
void* create_memory(const char* name, unsigned int size)
{
    int shm_id = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_id == -1) return NULL;
    if (ftruncate(shm_id, size) == -1) return NULL;
    void* ans = mmap(0, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);
    return (ans == MAP_FAILED) ? NULL : ans;
}
int delete_memory(const char* name) { return (shm_unlink(name) == -1) ? -1 : 0; }



#include "../log/log.h"
struct State init_state(const char* logfile)
{
    struct State state = { 0, 0, 0, 0, NULL };
    if (set_log_file(logfile) == -1) return state;

    state.door_in = create_semaphore(door_in_semaphore, 0);
    if (state.door_in == 0) return state;

    state.door_out = create_semaphore(door_out_semaphore, 0);
    if (state.door_out == 0) return state;

    state.reception_in = create_semaphore(reception_in_semaphore, 0);
    if (state.reception_in == 0) return state;

    state.reception_out = create_semaphore(reception_out_semaphore, 0);
    if (state.reception_out == 0) return state;

    state.shared_memory = create_memory(memory, sizeof(struct Message));
    if (state.shared_memory == NULL) return state;
    
    return state;
}

int close_state(struct State state)
{
    int status = 0;
    if (stop_logging() == -1) status = -1;
    if (close_semaphore(state.door_in) == -1) status = -1;
    if (close_semaphore(state.door_out) == -1) status = -1;
    if (close_semaphore(state.reception_in) == -1) status = -1;
    if (close_semaphore(state.reception_out) == -1) status = -1;
    return status;
}

#include <stdio.h>
int clear_state(struct State state)
{
    int status = 0;
    if (close_state(state) == -1) status = -1;
    if (delete_semaphore(door_in_semaphore) == -1) status = -1;
    if (delete_semaphore(door_out_semaphore) == -1) status = -1;
    if (delete_semaphore(reception_in_semaphore) == -1) status = -1;
    if (delete_semaphore(reception_out_semaphore) == -1) status = -1;
    if (delete_memory(memory) == -1) status = -1;
    return status;
}



#define TIMEOUT 250
int request(struct State state, struct Message message, struct Message* response)
{    
    if (wait_semaphore(state.door_in) == -1) return -1;// Wait for the hotel to accept the connection
    sleep_milliseconds(TIMEOUT); // Timeout to make the connection more realistic
    *(state.shared_memory) = message; // Put the message to the shared memory
    if (post_semaphore(state.reception_in) == -1) return -1; // Send the request
    
    if (wait_semaphore(state.reception_out) == -1) return -1; // Receive the response
    sleep_milliseconds(TIMEOUT); // Timeout to make the connection more realistic
    *response = *(state.shared_memory);
    if (post_semaphore(state.door_out) == -1) return -1; // Close the connection
    return 0;
}

int response(struct State state, struct Message message)
{
    sleep_milliseconds(TIMEOUT); // Timeout to make the connection more realistic
    (*state.shared_memory) = message; // Put the response to the shared memory
    if (post_semaphore(state.reception_out) == -1) return -1; // Tell the client that the response has been created
    if (wait_semaphore(state.door_out) == -1) return -1; // Wait for the client to read the response
    return 0;
}
