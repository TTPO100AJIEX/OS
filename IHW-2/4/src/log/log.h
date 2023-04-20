#pragma once

#include <stdio.h>
#include <semaphore.h>

extern FILE* log_file;
extern sem_t* log_semaphore;

int set_log_file(char* filepath);
int stop_logging();

void log_time();

#define log(template, ...)  sem_wait(log_semaphore); log_time(); fprintf(log_file, template, __VA_ARGS__); sem_post(log_semaphore)