#include "log.h"

#include <unistd.h>

FILE* log_file = NULL;
sem_t* log_semaphore = NULL;
static pid_t ownerPID;

int set_log_file(const char* filepath)
{
    if (log_semaphore)
    {
        if (stop_logging() == -1) return -1;
    }
    log_semaphore = create_semaphore("log_semaphore", 1);
    if (log_semaphore == NULL) return -1;
    ownerPID = getpid();

    log_file = fopen(filepath, "w");
    if (log_file == NULL) return -1;
    setbuf(log_file, NULL); // Remove buffering of output
    return 0;
}

int stop_logging()
{
    int status = 0;
    if (fclose(log_file) == -1) status = -1;
    if (close_semaphore(log_semaphore) == -1) status = -1;
    if (getpid() == ownerPID)
    {
        if (delete_semaphore("log_semaphore") == -1) status = -1;
    }
    return status;
}


#include <time.h>
#include <sys/time.h>
int log_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1;

    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec));
    fprintf(log_file, "[%s.%03ld.%03ld] ", buffer, tv.tv_usec / 1000, tv.tv_usec % 1000);
    return 0;
}