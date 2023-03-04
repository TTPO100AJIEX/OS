#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

const struct timespec threeHalvesSecond = { 1, 5e8 };
const struct timespec second = { 1, 0 };

int main(int argc, char** argv)
{
    int shm_id = shm_open("os-hw-7", O_CREAT | O_RDWR, 0666); // Create the memory
    if(shm_id == -1) { perror("Failed to create a shared memory instance"); exit(1); } // Throw an error if something went wrong

    if(ftruncate(shm_id, 8) == -1) { perror("Failed to size the shared memory"); exit(2); } // Set the size of the memory and throw an error if something went wrong

    int* addr = mmap(0, 4, PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0); // Load the memory and get its pointer in the address space of this program
    if(addr == MAP_FAILED) { perror("Failed to load the shared memory"); exit(3); } // Throw an error if something went wrong

    *(addr + 4) = 0; // Unset the flag to let the client know the server is working
    nanosleep(&threeHalvesSecond, NULL); // Wait 1.5 seconds to let the client connect and generate the first number
    while(*(addr + 4) == 1)
    {
        *(addr + 4) = 0; // Unset the flag
        printf("%d\n", *addr); // Read a number from memory and print it to the console
        nanosleep(&second, NULL); // Wait one second
    }

    if (shm_unlink("os-hw-7") == -1) { perror("Failed to delete the shared memory"); exit(4); } // Delete the memory and throw an error if something went wrong

    return 0;
}
