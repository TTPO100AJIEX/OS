#include "log.h"

#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

struct Logger initialize_logger()
{
    // Create the object
    return (struct Logger){ .owner = getpid() };
}
int delete_logger(struct Logger* logger)
{
    // Both parent and children do not need to do anything
    if (logger->owner != getpid()) return 0;
    return 0;
}


static char* write_integer(char* dest, int number)
{
    // Convert number into string
    if (number < 0)
    {
        *(dest++) = '-';
        return write_integer(dest, -number);
    }
    unsigned int pow10 = 1;
    while (number / pow10 >= 10) pow10 *= 10;
    while (pow10 != 0)
    {
        *(dest++) = '0' + (number / pow10);
        number %= pow10;
        pow10 /= 10;
    }
    return dest;
}
static char* write_uinteger(char* dest, unsigned int number, unsigned int digits)
{
    // Convert number into string with given amount of digits
    if (digits == 0) return dest;
    int pow10 = 1;
    for (unsigned int i = 0; i < digits - 1; i++) pow10 *= 10;
    *(dest++) = '0' + (number / pow10);
    return write_uinteger(dest, number % pow10, digits - 1);
}

#include <stdio.h>
void log_string(__attribute__ ((unused)) struct Logger* logger, const char* message)
{
    // Just print the message to the console
    printf("%s", message);
}

void log_integer(struct Logger* logger, int number)
{
    // Transform an integer into a string and log it
    char buffer[16]; *(write_integer(buffer, number)) = '\0';
    log_string(logger, buffer);
}
void log_uinteger(struct Logger* logger, unsigned int number, unsigned int digits)
{
    // Transform an integer into a string with given number of digits and log it
    char buffer[16]; *(write_uinteger(buffer, number, digits)) = '\0';
    log_string(logger, buffer);
}
void log_message(struct Logger* logger, const char* message)
{
    // Log a string with time
    log_time(logger);
    log_string(logger, message);
}
void log_layout(struct Logger* logger, struct Rooms* rooms)
{
    char* rooms_layout = get_rooms_layout(rooms); // Get the layout of the rooms
    log_string(logger, rooms_layout); // Log the layout
    free(rooms_layout); // Free the memory allocated by get_rooms_layout
}

void log_time(struct Logger* logger)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return; // Get current time up to microseconds
    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec)); // Parse current time into nice string

    // Some drawing :)
    char message[100]; char* writer = message;
    *(writer++) = '[';
    strcpy(writer, buffer);
    while (*writer != '\0') writer++;
    *(writer++) = '.';
    writer = write_uinteger(writer, tv.tv_usec / 1000, 3);
    *(writer++) = '.';
    writer = write_uinteger(writer, tv.tv_usec % 1000, 3);
    *(writer++) = ']'; *(writer++) = ' '; *(writer++) = '\0';

    log_string(logger, message);
}
void log_pid(struct Logger* logger)
{
    // Convert the PID into a string and log it
    char string[16] = " (pid: "; char* writer = string;
    while (*writer != '\0') writer++;
    writer = write_integer(writer, getpid());
    *(writer++) = ')'; *(writer++) = '\n';
    log_string(logger, string);
}