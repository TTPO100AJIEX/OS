#pragma once

void sleep_milliseconds(unsigned int milliseconds);


#include "protocol.h"
#include <semaphore.h>
#define door_in_semaphore "door_in_semaphore"
#define door_out_semaphore "door_out_semaphore"
#define reception_in_semaphore "reception_in_semaphore"
#define reception_out_semaphore "reception_out_semaphore"
#define memory "memory"

sem_t* create_semaphore(const char* name, unsigned int value);
int wait_semaphore(sem_t* sem);
int post_semaphore(sem_t* sem);
int close_semaphore(sem_t* sem);
int delete_semaphore(const char* name);

void* create_memory(const char* name, unsigned int size);
int delete_memory(const char* name);



struct State
{
    sem_t* door_in;
    sem_t* door_out;
    sem_t* reception_in;
    sem_t* reception_out;
    struct Message* shared_memory;
};
struct State init_state(const char* logfile);
int close_state(struct State state);
int clear_state(struct State state);



int request(struct State state, struct Message message, struct Message* response);
int response(struct State state, struct Message message);