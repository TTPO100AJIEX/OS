#pragma once

#include <semaphore.h>
sem_t* create_semaphore(const char* name, unsigned int value);
int wait_semaphore(sem_t* sem);
int post_semaphore(sem_t* sem);
int close_semaphore(sem_t* sem);
int delete_semaphore(const char* name);