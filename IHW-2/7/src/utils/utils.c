#include "utils.h"


#include <time.h>
void sleep_milliseconds(unsigned int milliseconds)
{
    struct timespec interval = { milliseconds / 1000, (milliseconds % 1000) * 1000000 };
    nanosleep(&interval, NULL);
}



#include <fcntl.h>
// Semaphore utilities
sem_t* create_semaphore(const char* name, unsigned int value)
{
    sem_t* sem = sem_open(name, O_CREAT, 0666, value);
    return (sem == SEM_FAILED) ? NULL : sem;
}
int wait_semaphore(sem_t* sem) { return (sem_wait(sem) == -1) ? -1 : 0; }
int post_semaphore(sem_t* sem) { return (sem_post(sem) == -1) ? -1 : 0; }
int close_semaphore(sem_t* sem) { return (sem && sem_close(sem) == -1) ? -1 : 0; }
int delete_semaphore(const char* name) { return (sem_unlink(name) == -1) ? -1 : 0; }


#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
// Shared memory utilities
void* create_memory(const char* name, unsigned int size)
{
    int shm_id = shm_open(name, O_CREAT | O_RDWR, 0666); // Create the memory
    if (shm_id == -1) return NULL;
    if (ftruncate(shm_id, size) == -1) return NULL; // Size the memory
    void* ans = mmap(0, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0); // Load the memory
    return (ans == MAP_FAILED) ? NULL : ans;
}
int write_memory(void* mem, const void* src, unsigned int size) { memcpy(mem, src, size); return 0; } // Just copy the data
int read_memory(const void* mem, void* dest, unsigned int size) { memcpy(dest, mem, size); return 0; } // Just copy the data
int close_memory(__attribute__ ((unused)) void* name) { return 0; } // Posix shared memory does not need to be closed
int delete_memory(const char* name) { return (shm_unlink(name) == -1) ? -1 : 0; }


// Utilities to work with state
int init_state(struct State* state)
{
    // Create the needed semaphores
    state->door_in = create_semaphore(door_in_semaphore, 0);
    if (state->door_in == NULL) return -1;

    state->reception_in = create_semaphore(reception_in_semaphore, 0);
    if (state->reception_in == NULL) return -1;

    state->reception_out = create_semaphore(reception_out_semaphore, 0);
    if (state->reception_out == NULL) return -1;

    state->door_out = create_semaphore(door_out_semaphore, 0);
    if (state->door_out == NULL) return -1;

    // Create the memory
    state->shared_memory = create_memory(message_memory, sizeof(struct Message));
    if (state->shared_memory == NULL) return -1;
    
    state->hotel_pid = create_memory(hotel_memory, sizeof(pid_t));
    if (state->hotel_pid == NULL) return -1;

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
    if (delete_semaphore(door_in_semaphore) == -1) status = -1;
    if (delete_semaphore(reception_in_semaphore) == -1) status = -1;
    if (delete_semaphore(reception_out_semaphore) == -1) status = -1;
    if (delete_semaphore(door_out_semaphore) == -1) status = -1;
    // Delete the memory
    if (delete_memory(message_memory) == -1) status = -1;
    if (delete_memory(hotel_memory) == -1) status = -1;
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
