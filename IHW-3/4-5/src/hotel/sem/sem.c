#include "sem.h"

#include <stddef.h>
#include <fcntl.h>

sem_t* create_semaphore(const char* name, unsigned int value)
{
    sem_t* sem = sem_open(name, O_CREAT, 0666, value);
    return (sem == SEM_FAILED) ? NULL : sem;
}
int wait_semaphore(sem_t* sem) { return (sem_wait(sem) == -1) ? -1 : 0; }
int post_semaphore(sem_t* sem) { return (sem_post(sem) == -1) ? -1 : 0; }
int close_semaphore(sem_t* sem) { return (sem && sem_close(sem) == -1) ? -1 : 0; }
int delete_semaphore(const char* name) { return (sem_unlink(name) == -1) ? -1 : 0; }
