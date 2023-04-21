#include "log.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

FILE* log_file;
sem_t* log_semaphore = NULL;
static pid_t ownerPID;

int set_log_file(char* filepath)
{
    if (log_semaphore) stop_logging();

    log_semaphore = sem_open("log_semaphore", O_CREAT, 0666, 1);
    if (log_semaphore == SEM_FAILED) return -1;
    ownerPID = getpid();

    log_file = fopen(filepath, "w");
    if (log_file == NULL) return -1;
    setbuf(log_file, NULL); // Remove buffering of output
    return 0;
}

int stop_logging()
{
    if (fclose(log_file) == -1) return -1;
    if (sem_close(log_semaphore) == -1) return -1;
    if (getpid() == ownerPID)
    {
        if (sem_unlink("log_semaphore") == -1) return -1;
    }
    return 0;
}


#include <time.h>
#include <sys/time.h>
int log_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1;

    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec));
    fprintf(log_file, "[%s.%03d.%03d] ", buffer, tv.tv_usec / 1000, tv.tv_usec % 1000);
    return 0;
}