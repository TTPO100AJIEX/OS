#include "utils.h"


#include <time.h>
void sleep_milliseconds(unsigned int milliseconds)
{
    struct timespec interval = { milliseconds / 1000, (milliseconds % 1000) * 1000000 };
    nanosleep(&interval, NULL);
}



#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define ipc_filepath "6/src/utils/utils.c"
// Semaphore utilities
static int string_hash(const char* string)
{
    int ans = 0;
    for (unsigned int i = 0; i < strlen(string); i++) ans = ((ans * 131) + string[i]) % 15999989;
    return ans;
}
int create_semaphore(const char* name, unsigned int value)
{
    int sem = semget(ftok(ipc_filepath, string_hash(name)), 1, 0666 | IPC_CREAT);
    if (sem == -1) return -1;
    for (unsigned int i = 0; i < value; i++)
    {
        if (post_semaphore(sem) == -1) return -1; // Set the initial value as System V does not allow to do that in semget
    }
    return sem;
}
int wait_semaphore(int sem)
{
    struct sembuf operation = { 0, -1, 0 };
    return semop(sem, &operation, 1);
}
int post_semaphore(int sem)
{
    struct sembuf operation = { 0, 1, 0 };
    return semop(sem, &operation, 1);
}
int close_semaphore(__attribute__ ((unused)) int sem) { return 0; }
int delete_semaphore(int sem) { return (semctl(sem, 1, IPC_RMID) == -1) ? -1 : 0; }



#include <sys/msg.h>
struct MessageQueueBuffer
{
    long mtype;
    char mtext[32]; // Definitely enough for both struct Message and pid_t that may be transferred
};
int create_memory(const char* name) { return msgget(IPC_PRIVATE + 1 + ftok(ipc_filepath, string_hash(name)), 0666 | IPC_CREAT); }
int write_memory(int msq, const void* src, unsigned int size)
{
    struct MessageQueueBuffer buffer;
    buffer.mtype = 1; // Might have been used more smartly, but as I want to transfer both struct Message and pid_t, it would have required some messy conditions
    memcpy(buffer.mtext, src, size); // Copy the data into the buffer
    return (msgsnd(msq, &buffer, size, 0) == -1) ? -1 : 0;
}
int read_memory(int msq, void* dest, unsigned int size)
{
    struct MessageQueueBuffer buffer;
    if (msgrcv(msq, &buffer, size, 1, 0) != size) return -1;
    memcpy(dest, buffer.mtext, size); // Copy the data to the destination
    return 0;
}
int close_memory(__attribute__ ((unused)) int msq) { return 0; } // System V message queues do not need to be closed
int delete_memory(int msq) { return (msgctl(msq, IPC_RMID, NULL) == -1) ? -1 : 0; }


#include <sys/types.h>
// Utilities to work with state
int init_state(struct State* state)
{
    // Create the needed semaphores
    state->door_in = create_semaphore(door_in_semaphore, 0);
    if (state->door_in == -1) return -1;

    state->reception_in = create_semaphore(reception_in_semaphore, 0);
    if (state->reception_in == -1) return -1;

    state->reception_out = create_semaphore(reception_out_semaphore, 0);
    if (state->reception_out == -1) return -1;

    state->door_out = create_semaphore(door_out_semaphore, 0);
    if (state->door_out == -1) return -1;

    // Create the memory
    state->shared_memory = create_memory(message_memory);
    if (state->shared_memory == -1) return -1;
    
    state->hotel_pid = create_memory(hotel_memory);
    if (state->hotel_pid == -1) return -1;

    return 0;
}

int close_state(struct State state)
{
    int status = 0;
    // Close the semaphores
    if (close_semaphore(state.door_in) == -1) status = -1;
    if (close_semaphore(state.door_out) == -1) status = -1;
    if (close_semaphore(state.reception_in) == -1) status = -1;
    if (close_semaphore(state.reception_out) == -1) status = -1;
    // Close the memory
    if (close_memory(state.shared_memory) == -1) status = -1;
    if (close_memory(state.hotel_pid) == -1) status = -1;
    return status;
}

int clear_state(struct State state)
{
    int status = 0;
    // Close everything
    if (close_state(state) == -1) status = -1;
    // Delete the semaphores
    if (delete_semaphore(state.door_in) == -1) status = -1;
    if (delete_semaphore(state.reception_in) == -1) status = -1;
    if (delete_semaphore(state.reception_out) == -1) status = -1;
    if (delete_semaphore(state.door_out) == -1) status = -1;
    // Delete the memory
    if (delete_memory(state.shared_memory) == -1) status = -1;
    if (delete_memory(state.hotel_pid) == -1) status = -1;
    return status;
}



#define TIMEOUT 250 // Timeout to make the connection more realistic
// Utilities to do the communication
int request(struct State state, struct Message message)
{    
    if (wait_semaphore(state.door_in) == -1) return -1; // Wait for the hotel to accept the connection
    sleep_milliseconds(TIMEOUT);
    write_memory(state.shared_memory, &message, sizeof(struct Message)); // Put the message to the shared memory
    if (post_semaphore(state.reception_in) == -1) return -1; // Send the request
    return 0;
}
int get_response(struct State state, struct Message* response)
{
    if (wait_semaphore(state.reception_out) == -1) return -1; // Wait for the response
    sleep_milliseconds(TIMEOUT);
    read_memory(state.shared_memory, response, sizeof(struct Message)); // Read the message from the shared memory
    if (post_semaphore(state.door_out) == -1) return -1; // Close the connection
    return 0;
}

int get_request(struct State state, struct Message* response)
{
    if (post_semaphore(state.door_in) == -1) return -1; // Let one client in
    if (wait_semaphore(state.reception_in) == -1) return -1; // Wait for the client to put the message into the shared memory
    sleep_milliseconds(TIMEOUT);
    read_memory(state.shared_memory, response, sizeof(struct Message)); // Read the message from the shared memory
    return 0;
}
int response(struct State state, struct Message message)
{
    sleep_milliseconds(TIMEOUT);
    write_memory(state.shared_memory, &message, sizeof(struct Message)); // Put the response to the shared memory
    if (post_semaphore(state.reception_out) == -1) return -1; // Tell the client that the response has been created
    if (wait_semaphore(state.door_out) == -1) return -1; // Wait for the client to read the response
    return 0;
}
