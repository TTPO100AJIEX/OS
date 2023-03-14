#include "messages.h"

#ifndef CHUNK_SIZE
    #error "CHUNK_SIZE must be defined"
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>

struct msgbuf
{
    long mtype;
    char mtext[CHUNK_SIZE + 1]; // Buffer for data. The first symbol is reserved for the algorithm
};

void input(const int input, const int output)
{
    struct msgbuf buffer = { 1, { 0 } };
    buffer.mtext[0] = 0;
    int lastRead = -1;
    while (lastRead != 0) // While there is something to read
    {
        lastRead = read(input, buffer.mtext + 1, CHUNK_SIZE); // Read data
        if (lastRead == -1) { perror("Input - read failed!"); return; } // Print an error if read failed
        if (msgsnd(output, &buffer, lastRead + 1, 0) == -1) { perror("Input - write failed!"); return; } // Put a message into the queue.
        // The last message will be empty (of length 1), which tells the other side that the data has ended
    }
}