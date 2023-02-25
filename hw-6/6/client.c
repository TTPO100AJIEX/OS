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
    int shm_id = shmget(ftok("./server.c", 1), getpagesize(), 0666); // Open the memory
    if(shm_id < 0) { perror("Failed to create a shared memory instance"); exit(1); } // Throw an error if something went wrong

    int* share = (int*)(shmat(shm_id, NULL, 0)); // "Connect" to the memory and get its pointer in the address space of this program
    if(share == NULL) { perror("Failed to link to shared memory"); exit(2); } // Throw an error if something went wrong

    srand(time(NULL)); // Set the seed
    while(true)
    {
        *share = random() % 1000; // Generate a random number and write it to the shared memory
        *(share + 4) = 1; // Set the flag
        nanosleep(&second, NULL); // Wait one second
    }
    return 0;
}
