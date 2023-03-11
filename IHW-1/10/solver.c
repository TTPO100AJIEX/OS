#include <stdio.h>
#include <sys/msg.h>
#include "messages.h"

int main(int argc, char** argv)
{
    int readerQueue = msgget(IPC_PRIVATE + 1, 0666);
    if (readerQueue == -1) { perror("Failed to open a reader message queue"); return -1; }
    int writerQueue = msgget(IPC_PRIVATE + 2, 0666);
    if (writerQueue == -1) { perror("Failed to open a writer message queue"); return -1; }
    
    solve(readerQueue, writerQueue);
}