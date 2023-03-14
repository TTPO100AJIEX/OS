#include "pipes.h"

#ifndef CHUNK_SIZE
    #error "CHUNK_SIZE must be defined"
#endif

#include <stdio.h>
#include <unistd.h>

void transfer(const int input, const int output)
{
    char inbuffer[CHUNK_SIZE]; // Buffer for data
    int lastRead = -1;
    while (lastRead != 0) // While there is something to read
    {
        lastRead = read(input, inbuffer, CHUNK_SIZE); // Read data
        if (lastRead == -1) { perror("Transfer - read failed!"); return; } // Print an error if read failed
        if (lastRead != 0)
        {
            // It is undefined behavior to write 0 bytes into a fifo, so the "if" is required
            if (write(output, inbuffer, lastRead) != lastRead) { perror("Transfer - write failed!"); return; } // If something was read, write it
        }
    }
}