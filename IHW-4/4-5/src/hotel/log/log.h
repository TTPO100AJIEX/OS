#pragma once

#include <stdbool.h>
#include <sys/types.h>
#include "../rooms/rooms.h"
struct Logger
{
    pid_t owner; // Owner of the logger (to determine who must delete everything)
    bool ok; // Has the logger been constructed successfully
};

struct Logger initialize_logger();
int delete_logger(struct Logger* this);


int log_string(struct Logger* this, const char* message);

int log_integer(struct Logger* this, int number);
int log_uinteger(struct Logger* this, unsigned int number, unsigned int digits);
int log_message(struct Logger* this, const char* message);
int log_layout(struct Logger* this, struct Rooms* rooms);

int log_time(struct Logger* this);
int log_pid(struct Logger* this);