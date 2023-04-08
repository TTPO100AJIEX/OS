#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semid;
int pipeDesc[2];
pid_t parentId, childId;
int sem_increase(int value)
{
    struct sembuf operation = { 0, value, 0 };
    return semop(semid, &operation, 1);
}

void parent()
{
    while (true)
    {
        sleep(2);

        int msg = rand() % 10;
        printf("Parent sent %d\n", msg);
        write(pipeDesc[1], &msg, sizeof(msg));

        sem_increase(2);
        sem_increase(0);

        read(pipeDesc[0], &msg, sizeof(msg));
        printf("Parent received %d\n\n", msg);
    }
}
void child()
{
    while (true)
    {
        sem_increase(-1);

        int msg;
        read(pipeDesc[0], &msg, sizeof(msg));
        printf("Child received %d\n", msg);
        
        sleep(1);

        msg <<= 1;
        printf("Child sent %d\n", msg);
        write(pipeDesc[1], &msg, sizeof(msg));
        
        sem_increase(-1);
    }
}

void stop(int sig)
{
    if (childId == 0)
    {
        if (sig == SIGINT)
        {
            if (kill(parentId, SIGTERM) == -1) { perror("Child: failed to kill the parent"); close(pipeDesc[0]); close(pipeDesc[1]); exit(-1); }
        }
        if (close(pipeDesc[0]) == -1) { perror("Child: failed to close the reading side of the pipe"); close(pipeDesc[1]); exit(-1); }
        if (close(pipeDesc[1]) == -1) { perror("Child: failed to close the writing side of the pipe"); exit(-1); }
        exit(0);
    }
    else
    {
        if (sig == SIGINT)
        {
            if (kill(childId, SIGTERM) == -1) { perror("Parent: failed to kill the child"); semctl(semid, 1, IPC_RMID); close(pipeDesc[0]); close(pipeDesc[1]); exit(-1); }
        }
        if (semctl(semid, 1, IPC_RMID) == -1) { perror("Parent: failed to delete a semaphore"); close(pipeDesc[0]); close(pipeDesc[1]); exit(-1); }
        if (close(pipeDesc[0]) == -1) { perror("Parent: failed to close the reading side of the pipe"); close(pipeDesc[1]); exit(-1); }
        if (close(pipeDesc[1]) == -1) { perror("Parent: failed to close the writing side of the pipe"); exit(-1); }
        exit(0);
    }
}

int main(void)
{
    srand(time(NULL));
    parentId = getpid();

    key_t key = ftok("index.c", 0);
    if (key == -1) { perror("Failed to generate a semaphore key"); return -1; }
    semid = semget(key, 1, 0666 | IPC_CREAT);
    if (semid == -1) { perror("Failed to create a semaphore"); return -1; }

    if (pipe(pipeDesc) == -1) { perror("Failed to create the pipe"); semctl(semid, 1, IPC_RMID); return -1; }
    
    childId = fork();
    if (childId == -1) { perror("Failed to create the child process"); semctl(semid, 1, IPC_RMID); close(pipeDesc[0]); close(pipeDesc[1]); return -1; }

    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    if (childId == 0) child();
    else parent();
}