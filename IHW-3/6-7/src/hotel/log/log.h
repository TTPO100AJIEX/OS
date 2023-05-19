#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "../sem/sem.h"
#include "../rooms/rooms.h"
struct Logger
{
    char* semaphore_name;
    sem_t* sync;

    char* message_queue_name;
    int message_queue;

    size_t amount;
    int* destinations;
    pid_t logger_pid;

    bool ok;
};

struct Logger initialize_log(const char* semaphore_name, const char* memory_name);
int close_log(struct Logger* logger);
int destroy_log(struct Logger* logger);

int lock_log(struct Logger* logger);
int unlock_log(struct Logger* logger);

int add_destination(struct Logger* logger, int fd);


void log_string(struct Logger* logger, const char* message);

void log_integer(struct Logger* logger, int number);
void log_uinteger(struct Logger* logger, unsigned int number, unsigned int digits);
void log_message(struct Logger* logger, const char* message);
void log_layout(struct Logger* logger, struct Rooms* rooms);

void log_time(struct Logger* logger);
void log_pid(struct Logger* logger);