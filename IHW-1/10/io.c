#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <signal.h>
#include "messages.h"

int main(int argc, char** argv)
{
    if (argc < 3) { printf("Not enough command line arguments specified"); return -1; }

    pid_t writer = fork();
    if (writer == -1) { perror("Failed to create the writer process"); return -1; }
    if (writer == 0)
    {
        int outputFile = open(argv[2], O_CREAT | O_WRONLY, 0666);
        if (outputFile == -1) { perror("Writer - failed to open an output file"); return -1; }
        if (ftruncate(outputFile, 0) == -1) { perror("Writer - failed to clear the output file"); close(outputFile); return -1; }

        int writerQueue = msgget(IPC_PRIVATE + 2, IPC_CREAT | 0666);
        if (writerQueue == -1) { perror("Writer - failed to create a writer message queue"); close(outputFile); return -1; }

        output(writerQueue, outputFile);

        if (close(outputFile) == -1) { perror("Writer - failed to close the output file"); msgctl(writerQueue, IPC_RMID, NULL); return -1; }
        if (msgctl(writerQueue, IPC_RMID, NULL) == -1) { perror("Writer - failed to delete the writer message queue"); return -1; }
        return 0;
    }

    int readerQueue = msgget(IPC_PRIVATE + 1, IPC_CREAT | 0666);
    if (readerQueue == -1) { perror("Failed to create a reader message queue"); kill(writer, SIGINT); return -1; }

    int inputFile = open(argv[1], O_RDONLY);
    if (inputFile == -1) { perror("Failed to open an input file"); kill(writer, SIGINT); msgctl(readerQueue, IPC_RMID, NULL); return -1; }

    input(inputFile, readerQueue);

    if (close(inputFile) == -1) { perror("Failed to close the input file"); kill(writer, SIGINT); msgctl(readerQueue, IPC_RMID, NULL); return -1; }

    waitpid(writer, NULL, 0);
    if (msgctl(readerQueue, IPC_RMID, NULL) == -1) { perror("Failed to delete the reader message queue"); return -1; }
}