#pragma once

#include <stdbool.h>
#include <sys/types.h>
#include "../utils/msq/msq.h"
#include "../utils/sem/sem.h"
#include "../rooms/rooms.h"
struct Logger
{
    pid_t owner; // Owner of the logger (to determine who must delete everything)
    bool ok; // Has the logger been constructed successfully
    
    struct MessageQueue msq; // Message queue to transfer messages to the parent thread
    struct Semaphore msqsem; // Semaphore to make the message queue contain not more than one message

    unsigned int destinations_amount; // Amount of the external loggers
    int* destinations; // A list of external loggers
};

struct Logger initialize_logger(const char* message_queue_name, const char* semaphore_name);
int delete_logger(struct Logger* this);

int add_log_destination(struct Logger* this, int client);

int read_string(struct Logger* this);


int log_string(struct Logger* this, const char* message);

int log_integer(struct Logger* this, int number);
int log_uinteger(struct Logger* this, unsigned int number, unsigned int digits);
int log_message(struct Logger* this, const char* message);
int log_layout(struct Logger* this, struct Rooms* rooms);

int log_time(struct Logger* this);
int log_pid(struct Logger* this);