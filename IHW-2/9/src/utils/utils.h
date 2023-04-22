#pragma once

// Sleep utility
void sleep_milliseconds(unsigned int milliseconds);


#include "protocol.h"
// Names of synchronization primitives used
#define door_in_semaphore "/door_in_semaphore"
#define door_out_semaphore "/door_out_semaphore"
#define reception_in_semaphore "/reception_in_semaphore"
#define reception_out_semaphore "/reception_out_semaphore"
#define message_memory "/message_memory"
#define hotel_memory "/hotel_memory"

// Semaphore utilities
#include <semaphore.h>
sem_t* create_semaphore(const char* name, unsigned int value);
int wait_semaphore(sem_t* sem);
int post_semaphore(sem_t* sem);
int close_semaphore(sem_t* sem);
int delete_semaphore(const char* name);

// Shared memory utilities
#include <mqueue.h>
mqd_t create_memory(const char* name, unsigned int size);
int write_memory(mqd_t msq, const void* src, unsigned int size);
int read_memory(mqd_t msq, void* dest, unsigned int size);
int close_memory(mqd_t msq);
int delete_memory(const char* name);


// The storage of all synchronization primitives used
#include <sys/types.h>
struct State
{
    sem_t* door_in; // Semaphore to send a request to the hotel
    sem_t* reception_in; // Semaphore to tell the hotel that the request has been put into the memory
    sem_t* reception_out; // Semaphore to tell the visitor that the response has been put into the memory
    sem_t* door_out; // Semaphore to tell the hotel that the response has been processed
    mqd_t shared_memory; // The memory to transfer messages
    mqd_t hotel_pid; // The memory to store the PID of the hotel
};
// Utilities to work with state
int init_state(struct State* state);
int close_state(struct State state);
int clear_state(struct State state);


// Utilities to do the communication
int request(struct State state, struct Message message);
int get_response(struct State state, struct Message* response);

int get_request(struct State state, struct Message* response);
int response(struct State state, struct Message message);