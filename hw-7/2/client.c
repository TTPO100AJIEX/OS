#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

const struct timespec halfSecond = { 0, 5e8 };
const struct timespec second = { 1, 0 };

int main(int argc, char** argv)
{
    int shm_id = shm_open("os-hw-7", O_CREAT | O_RDWR, 0666); // Create the memory
    if(shm_id == -1) { perror("Failed to create a shared memory instance"); exit(1); } // Throw an error if something went wrong

    if(ftruncate(shm_id, 4) == -1) { perror("Failed to size the shared memory"); exit(2); } // Set the size of the memory and throw an error if something went wrong

    int* addr = mmap(0, 4, PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);
    if(addr == MAP_FAILED) { perror("Failed to load the shared memory"); exit(3); } // Throw an error if something went wrong

    srand(time(NULL)); // Set the seed
    while(*addr < 0)
    {
        *addr = random() % 1000; // Generate a random number and write it to the shared memory
        printf("%d\n", *addr); // Print the number to the console because you asked to do it :)
        nanosleep(&second, NULL); // Wait one second
    }
    
    return 0;
}
