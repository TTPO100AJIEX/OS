#pragma once

#include <sys/types.h>
#include "../rooms/rooms.h"
struct Logger
{
    pid_t owner;
};

struct Logger initialize_logger();
int delete_logger(struct Logger* logger);


void log_string(struct Logger* logger, const char* message);

void log_integer(struct Logger* logger, int number);
void log_uinteger(struct Logger* logger, unsigned int number, unsigned int digits);
void log_message(struct Logger* logger, const char* message);
void log_layout(struct Logger* logger, struct Rooms* rooms);

void log_time(struct Logger* logger);
void log_pid(struct Logger* logger);