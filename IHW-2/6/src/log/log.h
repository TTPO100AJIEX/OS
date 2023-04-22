#pragma once

#include <stdio.h>
#include "../utils/utils.h"

extern FILE* log_file;
extern int log_semaphore;

int set_log_file(const char* filepath);
int stop_logging();

int log_time();

// Thread-safe logging: prints current time and message
#define log(template, ...)  if (wait_semaphore(log_semaphore) == -1) printf("Log: failed to wait on the semaphore\n"); \
                            if (log_time() == -1) printf("Log: failed to print the time\n"); \
                            if (fprintf(log_file, template __VA_OPT__(,) __VA_ARGS__) < 0) printf("Log: failed to print the message\n"); \
                            if (post_semaphore(log_semaphore) == -1) printf("Log: failed to open the semaphore\n")
