#pragma once

#include <sys/types.h>
struct Semaphore
{
    pid_t owner; // Owner of the semaphore
    int id; // ID of the instance
};

// Semaphore utilities
struct Semaphore create_semaphore(const char* name, unsigned int value);
int delete_semaphore(struct Semaphore* sem);

int wait_semaphore(struct Semaphore* sem);
int post_semaphore(struct Semaphore* sem);