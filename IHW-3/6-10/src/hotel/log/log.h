#pragma once

#include <sys/types.h>
#include "../rooms/rooms.h"
#include "../msq/msq.h"
#include "../sem/sem.h"
struct Logger
{
    pid_t owner;
    struct MessageQueue* msq;
    struct Semaphore* msqsem;

    unsigned int destinations_amount;
    int* destinations;
};

struct Logger initialize_logger(struct MessageQueue* msq, struct Semaphore* msqsem);
int delete_logger(struct Logger* logger);

int add_log_destination(struct Logger* logger, int client);


void log_string(struct Logger* logger, const char* message);

void log_integer(struct Logger* logger, int number);
void log_uinteger(struct Logger* logger, unsigned int number, unsigned int digits);
void log_message(struct Logger* logger, const char* message);
void log_layout(struct Logger* logger, struct Rooms* rooms);

void log_time(struct Logger* logger);
void log_pid(struct Logger* logger);