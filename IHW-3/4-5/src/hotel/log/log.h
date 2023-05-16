#pragma once

#include "../sem/sem.h"
extern sem_t* log_semaphore;
int initialize_log(const char* semaphore_name);
int close_log();
int destroy_log();

#include <stdio.h>
int log_time();
// Thread-safe logging: prints current time and message
#define log(template, ...)  if (wait_semaphore(log_semaphore) == -1) perror("Log: failed to wait on the semaphore"); \
                            if (log_time() == -1) perror("Log: failed to print the time"); \
                            if (printf(template __VA_OPT__(,) __VA_ARGS__) < 0) perror("Log: failed to print the message"); \
                            if (post_semaphore(log_semaphore) == -1) perror("Log: failed to open the semaphore")
