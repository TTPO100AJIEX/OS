// gcc index.c -o index.exe
// ./index.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semid; // ID of the semaphore
int pipeDesc[2]; // Descriptors of the pipe
pid_t parentId, childId; // PIDs of the processes
bool processEnded = false; // Flag whether the process has already ended
int sem_increase(int value) // Wrapper around semop
{
    struct sembuf operation = { 0, value, 0 }; // Create the operation struct
    return semop(semid, &operation, 1); // Execute the operation on the semaphore
}

void stop(int sig) // Signal handler
{
    if (processEnded) return; // If the process has already received a stop sig nal, don't do anything (to avoid recursive kills)
    if (childId == 0) // if the current process is a child
    {
        processEnded = true; // Set the flag
        // Kill the parent and throw if something failed
        if (kill(parentId, SIGINT) == -1) { perror("Child: failed to kill the parent"); close(pipeDesc[0]); close(pipeDesc[1]); exit(-1); }
        // Close both sides of the pipe and throw an error if something failed
        if (close(pipeDesc[0]) == -1) { perror("Child: failed to close the reading side of the pipe"); close(pipeDesc[1]); exit(-1); }
        if (close(pipeDesc[1]) == -1) { perror("Child: failed to close the writing side of the pipe"); exit(-1); }
        // Stop the process
        exit(0);
    }
    else // The current process is a parent
    {
        processEnded = true; // Set the flag
        // Kill the child and throw if something failed
        if (kill(childId, SIGINT) == -1) { perror("Parent: failed to kill the child"); semctl(semid, 1, IPC_RMID); close(pipeDesc[0]); close(pipeDesc[1]); exit(-1); }
        // Delete the semaphore and throw if the opreation failed
        if (semctl(semid, 1, IPC_RMID) == -1) { perror("Parent: failed to delete a semaphore"); close(pipeDesc[0]); close(pipeDesc[1]); exit(-1); }
        // Close both sides of the pipe and throw an error if something failed
        if (close(pipeDesc[0]) == -1) { perror("Parent: failed to close the reading side of the pipe"); close(pipeDesc[1]); exit(-1); }
        if (close(pipeDesc[1]) == -1) { perror("Parent: failed to close the writing side of the pipe"); exit(-1); }
        // Stop the process
        exit(0);
    }
}

void parent() // Parent process's logic
{
    while (true)
    {
        sleep(2);

        int msg = rand() % 10; // Generate a message
        printf("Parent sent %d\n", msg); // Print the message
        // Write the message to the pipe
        if (write(pipeDesc[1], &msg, sizeof(msg)) == -1) { perror("Parent: write failed"); stop(SIGINT); }

        // Add 2 to the semaphore
        if (sem_increase(2) == -1) { perror("Parent: semaphore increase failed"); stop(SIGINT); }
        // Wait for the child to do the processing (semaphore will get back to 0)
        if (sem_increase(0) == -1) { perror("Parent: semaphore wait failed"); stop(SIGINT); }

        if (read(pipeDesc[0], &msg, sizeof(msg)) == -1) { perror("Parent: read failed"); stop(SIGINT); } // Read the response from the pipe
        printf("Parent received %d\n\n", msg); // Print the response
    }
}
void child() // Child process's logic
{
    while (true)
    {
        // Wait for the parent to write to the pipe (and add 2 to the semaphore). Then subtract 1 from the semaphore's value
        if (sem_increase(-1) == -1) { perror("Child: semaphore wait failed"); stop(SIGINT); }

        int msg;
        if (read(pipeDesc[0], &msg, sizeof(msg)) == -1) { perror("Child: read failed"); stop(SIGINT); } // Read the message from the pipe
        printf("Child received %d\n", msg); // Print the message
        
        sleep(1);

        msg <<= 1; // Do some calculation (in this case, multiply by 2)
        printf("Child sent %d\n", msg); // Print the new value
        if (write(pipeDesc[1], &msg, sizeof(msg)) == -1) { perror("Parent: write failed"); stop(SIGINT); } // Write back to the pipe
        
        // Get the value of the semaphore back to 0 to indicate that the child process has done the processing
        if (sem_increase(-1) == -1) { perror("Child: semaphore decrease failed"); stop(SIGINT); }
    }
}

int main(void)
{
    srand(time(NULL)); // Set the seed for rand() to generate messages
    parentId = getpid(); // Save the PID of the parent process

    key_t key = ftok("index.c", 0); // Get the key for the semaphore
    if (key == -1) { perror("Failed to generate a semaphore key"); return -1; } // Throw error if ftok failed
    semid = semget(key, 1, 0666 | IPC_CREAT); // Create the semaphore with the generated key
    if (semid == -1) { perror("Failed to create a semaphore"); return -1; } // Throw error if semget failed

    // Create the pipe and throw an error (and delete the semaphore) if the opereation failed
    if (pipe(pipeDesc) == -1) { perror("Failed to create the pipe"); semctl(semid, 1, IPC_RMID); return -1; }
    
    childId = fork(); // Create the child process
    // Throw an error and close everything if fork failed
    if (childId == -1) { perror("Failed to create the child process"); semctl(semid, 1, IPC_RMID); close(pipeDesc[0]); close(pipeDesc[1]); return -1; }

    // Register signal handlers for both the child and the parent
    signal(SIGINT, stop);
    signal(SIGTERM, stop);

    if (childId == 0) child(); // Run the child
    else parent(); // Run the parent
}