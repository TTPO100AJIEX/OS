#pragma once

#include <stdio.h>
#include "../utils/utils.h"

extern FILE* log_file;
extern int log_semaphore;

int set_log_file(const char* filepath);
int stop_logging();

int log_time();

#define log(template, ...)  if (wait_semaphore(log_semaphore) == -1) perror("Log: failed to wait on the semaphore"); \
                            if (log_time() == -1) perror("Log: failed to print the time"); \
                            if (fprintf(log_file, template, __VA_ARGS__) < 0) perror("Log: failed to print the message"); \
                            if (post_semaphore(log_semaphore) == -1) perror("Log: failed to open the semaphore")
