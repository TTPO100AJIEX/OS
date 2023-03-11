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
    char mtext[CHUNK_SIZE + 1];
};

void output(const int input, const int output)
{
    struct msgbuf buffer = { 1 };
    int lastRead = -1;
    while (lastRead != 0)
    {
        lastRead = msgrcv(input, &buffer, CHUNK_SIZE + 1, 1, 0);
        if (lastRead == -1) { perror("Output - read failed"); return; }
        if (write(output, buffer.mtext, lastRead) != lastRead) { perror("Output - write failed"); return; }
    }
}