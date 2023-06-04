#include "sem.h"

#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>

static int string_hash(const char* string)
{
    int ans = 0;
    for (unsigned int i = 0; i < strlen(string); i++) ans = ((ans * 131) + string[i]) % 15999989;
    return ans;
}

struct Semaphore create_semaphore(const char* name, unsigned int value)
{
    key_t key = ftok(".", string_hash(name)); // Get a key
    semctl(semget(key, 1, 0), 1, IPC_RMID); // Delete old semaphore if exists
    // Create the semaphore
    struct Semaphore sem = { .owner = getpid(), .id = semget(key, 1, 0666 | IPC_CREAT) };
    if (sem.id == -1) return sem;
    // Set the initial value as System V does not allow to do that in semget
    struct sembuf operation = { 0, value, 0 };
    if (semop(sem.id, &operation, 1) == -1)
    {
        delete_semaphore(&sem);
        return (struct Semaphore){ .id = -1, .owner = getpid() }; 
    }
    return sem;
}
int delete_semaphore(struct Semaphore* sem)
{
    if (sem->owner != getpid()) return 0; // The semaphore does not need to be closed in every child process
    return semctl(sem->id, 1, IPC_RMID); // Delete the semaphore in the parent process
}


int wait_semaphore(struct Semaphore* sem)
{
    struct sembuf operation = { 0, -1, 0 };
    return semop(sem->id, &operation, 1);
}
int post_semaphore(struct Semaphore* sem)
{
    struct sembuf operation = { 0, 1, 0 };
    return semop(sem->id, &operation, 1);
}