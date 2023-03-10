#include "messages.h"

#ifndef CHUNK_SIZE
    #error "CHUNK_SIZE must be defined"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void transfer(const int input, const int output)
{
    char inbuffer[CHUNK_SIZE];
    int lastRead = -1;
    while (lastRead != 0)
    {
        lastRead = read(input, inbuffer, CHUNK_SIZE);
        if (lastRead == -1) { perror("Transfer - read failed!"); return; }
        if (write(output, inbuffer, lastRead) != lastRead) { perror("Transfer - write failed!"); return; }
    }
}