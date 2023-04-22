#pragma once

// Sleep utility
void sleep_milliseconds(unsigned int milliseconds);


#include "protocol.h"
// Names of synchronization primitives used
#define door_in_semaphore "/door_in_semaphore"
#define door_out_semaphore "/door_out_semaphore"
#define reception_in_semaphore "/reception_in_semaphore"
#define reception_out_semaphore "/reception_out_semaphore"
#define memory "/memory"

// Semaphore utilities
int create_semaphore(const char* name, unsigned int value);
int wait_semaphore(int sem);
int post_semaphore(int sem);
int close_semaphore(int sem);
int delete_semaphore(int sem);

// Shared memory utilities
void* create_memory(const char* name, unsigned int size);
int write_memory(void* mem, const void* src, unsigned int size);
int read_memory(const void* mem, void* dest, unsigned int size);
int close_memory(void* name);
int delete_memory(const char* name);


// The storage of all synchronization primitives used
struct State
{
    int door_in; // Semaphore to send a request to the hotel
    int reception_in; // Semaphore to tell the hotel that the request has been put into the memory
    int reception_out; // Semaphore to tell the visitor that the response has been put into the memory
    int door_out; // Semaphore to tell the hotel that the response has been processed
    struct Message* shared_memory; // The memory to transfer messages
};
// Utilities to work with state
int init_state(const char* logfile, struct State* state);
int close_state(struct State state);
int clear_state(struct State state);


// Utilities to do the communication
int request(struct State state, struct Message message);
int get_response(struct State state, struct Message* response);

int get_request(struct State state, struct Message* response);
int response(struct State state, struct Message message);