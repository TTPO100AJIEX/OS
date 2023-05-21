#include "log.h"

#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>

struct Logger initialize_logger(const char* message_queue_name, const char* semaphore_name)
{
    // Create the object
    struct Logger answer = {
        .owner = getpid(),
        .ok = false,
        
        .msq = create_message_queue(message_queue_name),
        .msqsem = create_semaphore(semaphore_name, 1),
        
        .destinations_amount = 0,
        .destinations = NULL
    };
    // Check if everything initialized successfully
    if (answer.msq.id != -1 && answer.msqsem.id != -1) answer.ok = true;
    return answer;
}
int delete_logger(struct Logger* this)
{
    int status = 0;
    // Close the clients, and it is fine if the operation fails
    for (unsigned int i = 0; i < this->destinations_amount; i++) close(this->destinations[i]);
    free(this->destinations); // Both parent and children have to free the memory
    // Delete the message queue and the semaphore
    if (delete_message_queue(&(this->msq)) == -1) status = -1;
    if (delete_semaphore(&(this->msqsem)) == -1) status = -1;
    return status;
}

int lock_logger(struct Logger* this) { return wait_semaphore(&(this->msqsem)); }
int unlock_logger(struct Logger* this) { return post_semaphore(&(this->msqsem)); }

int add_log_destination(struct Logger* this, int client)
{
    ++this->destinations_amount; // Increase the amount of destinations
    this->destinations = realloc(this->destinations, this->destinations_amount * sizeof(int)); // Reallocate the storage
    if (!this->destinations) { this->destinations_amount = 0; return -1; } // Throw an error if something went wrong
    this->destinations[this->destinations_amount - 1] = client; // Save the new destination in the end
    return 0;
}

#include <stdio.h>
#include <sys/socket.h>
int read_string(struct Logger* this)
{
    char message[1024]; // Buffer
    if (read_message_queue(&(this->msq), message, 1000) == -1) { unlock_logger(this); return -1; } // Read one message
    message[1023] = '\0'; // Null terminator to avoid infinite loop for sure
    printf("%s", message); // Print to the console
    for (unsigned int i = 0; i < this->destinations_amount; i++) send(this->destinations[i], message, strlen(message), MSG_NOSIGNAL); // Broadcast to the loggers
    return unlock_logger(this); // Let the next message be placed onto the queue
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

#include <signal.h>
int log_string(struct Logger* this, const char* message)
{
    // Wait for the queue to empty
    if (lock_logger(this) == -1) return -1;
    // Send the message to the queue
    if (write_message_queue(&(this->msq), message, strlen(message)) == -1) { unlock_logger(this); return -1; }
    // Send the signal to the parent process to make it print the message
    if (kill(this->owner, SIGUSR1) == -1) { unlock_logger(this); return -1; }
    return 0;
}

int log_integer(struct Logger* this, int number)
{
    // Transform an integer into a string and log it
    char buffer[16]; *(write_integer(buffer, number)) = '\0';
    return log_string(this, buffer);
}
int log_uinteger(struct Logger* this, unsigned int number, unsigned int digits)
{
    // Transform an integer into a string with given number of digits and log it
    char buffer[16]; *(write_uinteger(buffer, number, digits)) = '\0';
    return log_string(this, buffer);
}
int log_message(struct Logger* this, const char* message)
{
    // Log a string with time
    return (log_time(this) == -1 || log_string(this, message) == -1) ? -1 : 0;
}
int log_layout(struct Logger* this, struct Rooms* rooms)
{
    char* rooms_layout = get_rooms_layout(rooms); // Get the layout of the rooms
    int status = log_string(this, rooms_layout); // Log the layout
    free(rooms_layout); // Free the memory allocated by get_rooms_layout
    return status;
}

int log_time(struct Logger* this)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1; // Get current time up to microseconds
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

    return log_string(this, message);
}
int log_pid(struct Logger* this)
{
    // Convert the PID into a string and log it
    char string[32] = " (pid: "; char* writer = &string[7];
    writer = write_integer(writer, getpid());
    *(writer++) = ')'; *(writer++) = '\n'; *(writer++) = '\0';
    return log_string(this, string);
}