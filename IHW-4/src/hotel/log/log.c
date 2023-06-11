#include "log.h"

#include <stdio.h>
#include <stddef.h>

extern void log_string(const char* string);


#include <time.h>
#include <sys/time.h>
void log_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return; // Get current time up to microseconds
    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec)); // Parse current time into nice string
    char message[100];
    snprintf(message, 100, "[%s.%03d.%03d] ", buffer, (int)(tv.tv_usec / 1000), (int)(tv.tv_usec % 1000)); // Print current time up to microseconds
    log_string(message);
}


#include <stdlib.h>
#include "../rooms/rooms.h"
void log_layout() 
{
    char* layout = get_rooms_layout(); // Get the layout of the rooms
    log_string(layout); // Log the layout
    free(layout); // Free the memory allocated by get_rooms_layout
}


void log_message(const char* message)
{
    log_time();
    log_string(message);
}


#include <stdarg.h>
void log_parametric(const char* fmt, ...)
{
    va_list myargs;

    va_start(myargs, fmt);
    int size = vsnprintf(NULL, 0, fmt, myargs) + 1;
    va_end(myargs);

    va_start(myargs, fmt);
    char message[size];
    vsnprintf(message, size, fmt, myargs);
    log_message(message);
    va_end(myargs);
}