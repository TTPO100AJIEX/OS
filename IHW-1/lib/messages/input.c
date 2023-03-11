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

void input(const int input, const int output)
{
    struct msgbuf buffer = { 1 };
    buffer.mtext[0] = 0;
    int lastRead = -1;
    while (lastRead != 0)
    {
        lastRead = read(input, buffer.mtext + 1, CHUNK_SIZE);
        if (lastRead == -1) { perror("Input - read failed!"); return; }
        if (msgsnd(output, &buffer, lastRead + 1, 0) == -1) { perror("Input - write failed!"); return; }
    }
}