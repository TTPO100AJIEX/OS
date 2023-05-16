#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
sem_t* log_semaphore = NULL;
static char* semaphore_name = NULL;
int initialize_log(const char* sem_name)
{
    semaphore_name = malloc(strlen(sem_name) + 1);
    strcpy(semaphore_name, sem_name);
    log_semaphore = create_semaphore(semaphore_name, 1);
    return (log_semaphore ? 0 : -1);
}
int close_log()
{
    free(semaphore_name);
    return close_semaphore(log_semaphore);
}
int destroy_log()
{
    int err = 0;
    if (close_semaphore(log_semaphore) == -1) err = errno;
    if (delete_semaphore(semaphore_name) == -1) err = errno;
    free(semaphore_name);
    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}

#include <time.h>
#include <sys/time.h>
int log_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1; // Get current time up to microseconds
    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec)); // Parse current time into nice string
    printf("[%s.%03d.%03d] ", buffer, (int)(tv.tv_usec / 1000), (int)(tv.tv_usec % 1000)); // Print current time up to microseconds into the file
    return 0;
}