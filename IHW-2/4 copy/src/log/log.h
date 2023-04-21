#pragma once

#include <stdio.h>
#include <semaphore.h>

extern FILE* log_file;
extern sem_t* log_semaphore;

int set_log_file(char* filepath);
int stop_logging();

int log_time();

#define log(template, ...)  if (sem_wait(log_semaphore) == -1) perror("Log: failed to wait on the semaphore"); \
                            if (log_time() == -1) perror("Log: failed to print the time"); \
                            if (fprintf(log_file, template, __VA_ARGS__) < 0) perror("Log: failed to print the message"); \
                            if (sem_post(log_semaphore) == -1) perror("Log: failed to open the semaphore")