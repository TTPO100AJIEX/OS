#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <signal.h>
#include "messages.h"

int main(int argc, char** argv)
{
    int readerQueue = msgget(IPC_PRIVATE + 1, 0666);
    if (readerQueue == -1) { perror("Failed to create a reader message queue"); return -1; }
    int writerQueue = msgget(IPC_PRIVATE + 2, 0666);
    if (writerQueue == -1) { perror("Writer - failed to create a writer message queue"); msgctl(readerQueue, IPC_RMID, NULL); return -1; }
    
    solve(readerQueue, writerQueue);
}