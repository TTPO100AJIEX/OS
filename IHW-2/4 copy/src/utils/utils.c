#include "utils.h"

#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include "../log/log.h"

void sleep_milliseconds(unsigned int milliseconds)
{
    struct timespec interval = { milliseconds / 1000, (milliseconds % 1000) * 1000000 }, remaining;
    while (nanosleep(&interval, &remaining) == -1)
    {
        interval.tv_nsec = remaining.tv_nsec;
        interval.tv_sec = remaining.tv_sec;
    }
}


int request(struct State state, struct Message message, struct Message* response)
{
    if (sem_wait(state.door_in) == -1) return -1; // Wait for the hotel to accept the connection
    *(state.shared_memory) = message; // Put the message to the shared memory
    if (sem_post(state.reception_in) == -1) return -1; // Send the request

    if (sem_wait(state.reception_out) == -1) return -1; // Receive the response
    *response = *(state.shared_memory);
    if (sem_post(state.door_out) == -1) return -1; // Close the connection
    return 0;
}

int response(struct State state, struct Message message)
{
    (*state.shared_memory) = message; // Put the response to the shared memory
    if (sem_post(state.reception_out) == -1) return -1; // Tell the client that the response has been created
    if (sem_wait(state.door_out) == -1) return -1; // Wait for the client to read the response
    return 0;
}



struct State init_state()
{

}

void clear_state(struct State state)
{
    close_state(state);
    if (shm_unlink(memory) == -1) perror("Controller: failed to delete the shared memory");
    if (sem_unlink(door_in_semaphore) == -1) perror("Controller: failed to delete the door semaphore");
    if (sem_unlink(door_out_semaphore) == -1) perror("Controller: failed to delete the door semaphore");
    if (sem_unlink(reception_in_semaphore) == -1) perror("Controller: failed to delete the reception_in semaphore");
    if (sem_unlink(reception_out_semaphore) == -1) perror("Controller: failed to delete the reception_out semaphore");
}

void close_state(struct State state)
{
    if (sem_close(state.door_in) == -1) perror("Hotel: failed to close the door semaphore");
    if (sem_close(state.door_out) == -1) perror("Hotel: failed to close the door semaphore");
    if (sem_close(state.reception_in) == -1) perror("Hotel: failed to close the reception_in semaphore");
    if (sem_close(state.reception_out) == -1) perror("Hotel: failed to close the reception_out semaphore");
    if (stop_logging() == -1) perror("Hotel: failed to stop the logging");
}