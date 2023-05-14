#pragma once

#include <stdio.h>
#include "../utils/utils.h"

int log_time();
// Logging: prints current time and message
#define log(template, ...) if (log_time() == -1) printf("Log: failed to print the time\n"); printf(template __VA_OPT__(,) __VA_ARGS__)
