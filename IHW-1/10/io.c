#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <signal.h>
#include "messages.h"

int main(int argc, char** argv)
{
    // Check the amount of command line arguments
    if (argc < 3) { printf("Not enough command line arguments specified"); return -1; }

    // Create and open two message queues
    int readerQueue = msgget(IPC_PRIVATE + 1, IPC_CREAT | 0666);
    if (readerQueue == -1) { perror("Failed to create a reader message queue"); return -1; }
    int writerQueue = msgget(IPC_PRIVATE + 2, IPC_CREAT | 0666);
    if (writerQueue == -1) { perror("Writer - failed to create a writer message queue"); msgctl(readerQueue, IPC_RMID, NULL); return -1; }


    // Run the writer
    pid_t writer = fork();
    if (writer == -1) { perror("Failed to create the writer process"); msgctl(readerQueue, IPC_RMID, NULL); msgctl(writerQueue, IPC_RMID, NULL); return -1; }
    if (writer == 0)
    {
        // Open the output file
        int outputFile = open(argv[2], O_CREAT | O_WRONLY, 0666);
        if (outputFile == -1) { perror("Writer - failed to open an output file"); return -1; }
        // Clear the file
        if (ftruncate(outputFile, 0) == -1) { perror("Writer - failed to clear the output file"); close(outputFile); return -1; }

        // Transfer the answer from the writer message queue into the file
        output(writerQueue, outputFile);

        // Close the file
        if (close(outputFile) == -1) { perror("Writer - failed to close the output file"); return -1; }
        return 0;
    }

    // Open the input file
    int inputFile = open(argv[1], O_RDONLY);
    if (inputFile == -1) { perror("Failed to open an input file"); kill(writer, SIGINT); msgctl(readerQueue, IPC_RMID, NULL); msgctl(writerQueue, IPC_RMID, NULL); return -1; }

    // Read from file into the message queue
    input(inputFile, readerQueue);

    // Close the file
    if (close(inputFile) == -1) { perror("Failed to close the input file"); kill(writer, SIGINT); msgctl(readerQueue, IPC_RMID, NULL); msgctl(writerQueue, IPC_RMID, NULL); return -1; }

    // Wait for the writer
    waitpid(writer, NULL, 0);
    
    // Delete the message queues
    if (msgctl(readerQueue, IPC_RMID, NULL) == -1) { perror("Failed to delete the reader message queue"); msgctl(writerQueue, IPC_RMID, NULL); return -1; }
    if (msgctl(writerQueue, IPC_RMID, NULL) == -1) { perror("Writer - failed to delete the writer message queue"); return -1; }
}