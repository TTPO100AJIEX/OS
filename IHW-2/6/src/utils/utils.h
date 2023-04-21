#pragma once

void sleep_milliseconds(unsigned int milliseconds);


#include "protocol.h"
#define door_in_semaphore "door_in_semaphore"
#define door_out_semaphore "door_out_semaphore"
#define reception_in_semaphore "reception_in_semaphore"
#define reception_out_semaphore "reception_out_semaphore"
#define memory "memory"

int create_semaphore(const char* name, unsigned int value);
int wait_semaphore(int sem);
int post_semaphore(int sem);
int close_semaphore(int sem);
int delete_semaphore(const char* name);

void* create_memory(const char* name, unsigned int size);
int delete_memory(const char* name);



struct State
{
    int door_in;
    int door_out;
    int reception_in;
    int reception_out;
    struct Message* shared_memory;
};
struct State init_state(const char* logfile);
int close_state(struct State state);
int clear_state(struct State state);



int request(struct State state, struct Message message, struct Message* response);
int response(struct State state, struct Message message);