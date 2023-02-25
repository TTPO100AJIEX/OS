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

    int shm_id = shmget(ftok("./server.c", 1), getpagesize(), 0666); // Open the memory
    if(shm_id < 0) { perror("Failed to create a shared memory instance"); exit(1); } // Throw an error if something went wrong

    int* share = (int*)(shmat(shm_id, NULL, 0)); // "Connect" to the memory and get its pointer in the address space of this program
    if(share == NULL) { perror("Failed to link to shared memory"); exit(2); } // Throw an error if something went wrong

    srand(time(NULL)); // Set the seed
    while(true)
    {
        *share = random() % atoi(argv[1]); // Generate a random number and write it to the shared memory
        if (*share >= 1000) exit(0); // Stop the client if an invalid number has been generated
        nanosleep(&second, NULL); // Wait one second
    }
    return 0;
}
