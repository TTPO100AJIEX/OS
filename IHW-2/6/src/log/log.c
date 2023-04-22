#include "log.h"

#include <unistd.h>
#include <sys/types.h>

FILE* log_file = NULL;
int log_semaphore = -1;
static pid_t ownerPID;

int set_log_file(const char* filepath)
{
    if (log_semaphore)
    {
        // Close the old logging
        if (stop_logging() == -1) return -1;
    }
    // Initialize the semaphore for synchronization
    log_semaphore = create_semaphore("/log_semaphore", 1);
    if (log_semaphore == -1) return -1;
    ownerPID = getpid(); // Save the owner of the logger: only he can delete everything

    log_file = fopen(filepath, "w"); // Open the file to log into
    if (log_file == NULL) return -1;
    setbuf(log_file, NULL); // Remove buffering of output
    return 0;
}

int stop_logging()
{
    int status = 0;
    if (log_file && fclose(log_file) == -1) status = -1; // Close the file
    if (close_semaphore(log_semaphore) == -1) status = -1; // Close the semaphore
    if (getpid() == ownerPID)
    {
        // If called by the owner, also delete the semaphore
        if (delete_semaphore(log_semaphore) == -1) status = -1;
    }
    return status;
}


#include <time.h>
#include <sys/time.h>
int log_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1; // Get current time up to microseconds
    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec)); // Parse current time into nice string
    fprintf(log_file, "[%s.%03d.%03d] ", buffer, (int)(tv.tv_usec / 1000), (int)(tv.tv_usec % 1000)); // Print current time up to microseconds into the file
    return 0;
}