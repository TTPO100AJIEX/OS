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
    if(argc < 2) { printf("Not enough command line arguments specified"); exit(-1); } // Throw an error if something went wrong

    int shm_id = shmget(ftok("./server.c", 1), getpagesize(), 0666 | IPC_CREAT); // Create the memory
    if(shm_id < 0) { perror("Failed to create a shared memory instance"); exit(1); } // Throw an error if something went wrong

    int* share = (int*)(shmat(shm_id, NULL, 0)); // "Connect" to the memory and get its pointer in the address space of this program
    if(share == NULL) { perror("Failed to link to shared memory"); exit(2); } // Throw an error if something went wrong

    nanosleep(&halfSecond, NULL); // Wait half a second to let the client connect and generate the first number
    for (unsigned int time = 0; time < atoi(argv[1]); ++time) // Every iteration takes a second
    {
        nanosleep(&second, NULL); // Wait one second
        printf("%d\n", *share); // Read a number from memory and print it to the console
    }

    shmdt(share); // Disconnect from the memory
    if (shmctl(shm_id, IPC_RMID, NULL) < 0) { perror("Failed to delete a shared memory instance"); exit(2); } // Delete the memory and throw an error if something went wrong

    return 0;
}
