#pragma once

#include <stdio.h>
int log_time();
// Thread-safe logging: prints current time and message
#define log(template, ...)  if (log_time() == -1) perror("Log: failed to print the time"); \
                            if (printf(template __VA_OPT__(,) __VA_ARGS__) < 0) perror("Log: failed to print the message")
