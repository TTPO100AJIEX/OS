#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../msq/msq.h"

struct Logger initialize_log(const char* semaphore_name, const char* memory_name)
{
    // Initialize all fields and return { ok = false } if something failed
    struct Logger logger = { .ok = false };
    if (!(logger.semaphore_name = malloc(strlen(semaphore_name) + 1))) return logger;
    if (!(logger.sync = create_semaphore(semaphore_name, 1))) return logger;


    if (!(logger.destinations = create_memory(memory_name, 100 * sizeof(int)))) return logger;
    // Save memory and semaphore names
    strcpy(logger.memory_name, memory_name);
    strcpy(logger.semaphore_name, semaphore_name);
    // Initialize the destinations
    memset(logger.destinations, -1, 100 * sizeof(int));
    // Initialization completed successfully
    logger.ok = true;
    return logger;
}
int close_log(struct Logger* logger)
{
    // Free the memory
    free(logger->semaphore_name);
    free(logger->memory_name);
    // Close the semaphore
    return close_semaphore(logger->sync);
}
int destroy_log(struct Logger* logger)
{
    int err = 0; // Save the error for perror to work correctly with this function
    // Close all remaining destinations
    for (unsigned int i = 0; i < 100; i++)
    {
        if (logger->destinations[i] != -1) close(logger->destinations[i]);
    }
    // Close and delete the semaphore
    if (close_semaphore(logger->sync) == -1) err = errno;
    if (delete_semaphore(logger->semaphore_name) == -1) err = errno;
    // Delete the memory
    if (delete_memory(logger->memory_name) == -1) err = errno;
    // Free the memory occupied by memory and semaphore names
    free(logger->memory_name);
    free(logger->semaphore_name);

    // Restore the error if needed and return
    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}

int lock_log(struct Logger* logger) { return wait_semaphore(logger->sync); }
int unlock_log(struct Logger* logger) { return post_semaphore(logger->sync); }

int add_destination(struct Logger* logger, int fd)
{
    if (lock_log(logger) == -1) return -1;
    for (unsigned int i = 0; i < 100; i++) // Loop over the loggers storage
    {
        if (logger->destinations[i] == -1) // If there is an empty place for the logger
        {
            logger->destinations[i] = fd; // Save the descriptor of the new logger
            if (unlock_log(logger) == -1) return -1;
            return 1;
        }
    }
    if (unlock_log(logger) == -1) return -1;
    return 0;
}



static char* write_integer(char* dest, int number)
{
    if (number < 0)
    {
        *(dest++) = '-';
        return write_integer(dest, -number);
    }
    int pow10 = 1;
    while (number / pow10 >= 10) pow10 *= 10;
    *(dest++) = '0' + (number / pow10);
    if (pow10 != 1) return write_integer(dest, number % pow10);
    return dest;
}
static char* write_uinteger(char* dest, unsigned int number, unsigned int digits)
{
    if (digits == 0) return dest;
    int pow10 = 1;
    for (unsigned int i = 0; i < digits - 1; i++) pow10 *= 10;
    *(dest++) = '0' + (number / pow10);
    return write_uinteger(dest, number % pow10, digits - 1);
}

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
void log_string(struct Logger* logger, const char* message)
{
    printf(message);
    for (unsigned int i = 0; i < 100; i++)
    {
        if (logger->destinations[i] == -1) continue;
        // TODO: broadcast
    }
}

void log_integer(struct Logger* logger, int number)
{
    char buffer[16];
    *(write_integer(buffer, number)) = '\0';
    log_string(logger, buffer);
}
void log_uinteger(struct Logger* logger, unsigned int number, unsigned int digits)
{
    char buffer[16];
    *(write_uinteger(buffer, number, digits)) = '\0';
    log_string(logger, buffer);
}
void log_message(struct Logger* logger, const char* message)
{
    log_time(logger);
    log_string(logger, message);
}
void log_layout(struct Logger* logger, struct Rooms* rooms)
{
    char* rooms_layout = get_rooms_layout(rooms);
    log_string(logger, rooms_layout);
    free(rooms_layout);
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
    char string[16] = " (pid: "; char* writer = string;
    while (*writer != '\0') writer++;
    writer = write_integer(writer, getpid());
    *(writer++) = ')'; *(writer++) = '\n';
    log_string(logger, string);
}