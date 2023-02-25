#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>

const struct timespec halfSecond = { 0, 5e8 };
const struct timespec second = { 1, 0 };

int main(int argc, char** argv)
{
    int shm_id = shmget(ftok("./server.c", 1), getpagesize(), 0666 | IPC_CREAT); // Create the memory
    if(shm_id < 0) { perror("Failed to create a shared memory instance"); exit(1); } // Throw an error if something went wrong

    int* share = (int*)(shmat(shm_id, NULL, 0)); // "Connect" to the memory and get its pointer in the address space of this program
    if(share == NULL) { perror("Failed to link to shared memory"); exit(2); } // Throw an error if something went wrong

    nanosleep(&halfSecond, NULL); // Wait half a second to let the client connect and generate the first number
    nanosleep(&second, NULL); // Wait one second
    while(*(share + 4)) // While the flag in the shared memory is set
    {
        *(share + 4) = 0; // Unset the flag
        printf("%d\n", *share); // Print the current number
        nanosleep(&second, NULL); // Wait one second
    }

    shmdt(share); // Disconnect from the memory
    if (shmctl(shm_id, IPC_RMID, NULL) < 0) { perror("Failed to delete a shared memory instance"); exit(2); } // Delete the memory and throw an error if something went wrong

    return 0;
}
