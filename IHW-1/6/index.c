#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include "pipes.h"

int main(int argc, char** argv)
{
    // Check the amount of command line arguments
    if (argc < 3) { printf("Not enough command line arguments specified"); return -1; }

    // Create the pipes
    int readerPipe[2], writerPipe[2];
    if (pipe(readerPipe) == -1) { perror("Failed to create a reader pipe"); return -1; }
    if (pipe(writerPipe) == -1) { perror("Failed to create a writer pipe"); close(readerPipe[0]); close(readerPipe[1]); return -1; }


    // Run the io process
    pid_t io = fork();
    if (io == -1) { perror("Failed to create the io process"); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (io == 0)
    {
        // Close parts of pipes that are not needed
        if (close(writerPipe[1]) == -1) { perror("IO - failed to close the writing side of writer pipe"); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[0]); return -1; }
        if (close(readerPipe[0]) == -1) { perror("IO - failed to close the reading side of reader pipe"); close(readerPipe[1]); close(writerPipe[0]); return -1; }
        
        // Run the writer
        pid_t writer = fork();
        if (writer == -1) { perror("IO - failed to create the writer process"); close(readerPipe[1]); close(writerPipe[0]); return -1; }
        if (writer == 0)
        {
            // Close parts of pipes that are not needed
            if (close(readerPipe[1]) == -1) { perror("IO::Writer - failed to close the writing side of reader pipe"); close(writerPipe[0]); return -1; }

            // Open the output file
            int outputFile = open(argv[2], O_CREAT | O_WRONLY, 0666);
            if (outputFile == -1) { perror("IO::Writer - failed to open an output file"); close(writerPipe[0]); return -1; }
            // Clear the file
            if (ftruncate(outputFile, 0) == -1) { perror("IO::Writer - failed to clear the output file"); close(writerPipe[0]); close(outputFile); return -1; }

            // Transfer the answer from the writer pipe into the file
            transfer(writerPipe[0], outputFile);

            // Close everything
            if (close(writerPipe[0]) == -1) { perror("IO::Writer - failed to close the reading side of writer pipe"); close(outputFile); return -1; }
            if (close(outputFile) == -1) { perror("IO::Writer - failed to close the output file"); return -1; }
            return 0;
        }
        
        // Close parts of pipes that are not needed
        if (close(writerPipe[0]) == -1) { perror("IO - failed to close the reading side of writer pipe"); kill(writer, SIGINT); close(readerPipe[1]); return -1; }
        
        // Open the input file
        int inputFile = open(argv[1], O_RDONLY);
        if (inputFile == -1) { perror("IO - failed to open an input file"); kill(writer, SIGINT); close(readerPipe[1]); return -1; }

        // Read from file into pipe
        transfer(inputFile, readerPipe[1]);

        // Close everything
        if (close(readerPipe[1]) == -1) { perror("IO - failed to close the writing side of reader pipe"); kill(writer, SIGINT); close(inputFile); return -1; }
        if (close(inputFile) == -1) { perror("IO - failed to close the input file"); kill(writer, SIGINT); return -1; }

        // Wait for the writer
        waitpid(writer, NULL, 0);
        return 0;
    }
    

    // Run the solver
    pid_t solver = fork();
    if (solver == -1) { perror("Failed to create the solver process"); kill(io, SIGINT); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (solver == 0)
    {
        // Close parts of pipes that are not needed
        if (close(readerPipe[1]) == -1) { perror("Solver - failed to close the writing side of reader pipe"); close(readerPipe[0]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
        if (close(writerPipe[0]) == -1) { perror("Solver - failed to close the reading side of writer pipe"); close(readerPipe[0]); close(writerPipe[1]); return -1; }
        
        // Do the processing
        solve(readerPipe[0], writerPipe[1]);

        // Close everything
        if (close(readerPipe[0]) == -1) { perror("Solver - failed to close the reading side of reader pipe"); close(writerPipe[1]); return -1; }
        if (close(writerPipe[1]) == -1) { perror("Solver - failed to close the writing side of writer pipe"); return -1; }
        return 0;
    }


    // Close everything
    if (close(readerPipe[0]) == -1) { perror("Failed to close the reading side of reader pipe"); kill(io, SIGINT); kill(solver, SIGINT); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (close(readerPipe[1]) == -1) { perror("Failed to close the writing side of reader pipe"); kill(io, SIGINT); kill(solver, SIGINT); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (close(writerPipe[0]) == -1) { perror("Failed to close the reading side of writer pipe"); kill(io, SIGINT); kill(solver, SIGINT); close(writerPipe[1]);  return -1; }
    if (close(writerPipe[1]) == -1) { perror("Failed to close the writing side of writer pipe"); kill(io, SIGINT); kill(solver, SIGINT); return -1; }

    // Wait for all processes to do their job
    waitpid(io, NULL, 0);
    waitpid(solver, NULL, 0);
}