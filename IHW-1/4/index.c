#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include "pipes.h"

int main(int argc, char** argv)
{
    if (argc < 3) { printf("Not enough command line arguments specified"); return -1; }

    int readerPipe[2], writerPipe[2];
    if (pipe(readerPipe) == -1) { perror("Failed to create a reader pipe"); return -1; }
    if (pipe(writerPipe) == -1) { perror("Failed to create a writer pipe"); close(readerPipe[0]); close(readerPipe[1]); return -1; }
    
    
    pid_t reader = fork();
    if (reader == -1) { perror("Failed to create the reader process"); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (reader == 0)
    {
        if (close(writerPipe[0]) == -1) { perror("Reader - failed to close the reading side of writer pipe"); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[1]); return -1; }
        if (close(writerPipe[1]) == -1) { perror("Reader - failed to close the writing side of writer pipe"); close(readerPipe[0]); close(readerPipe[1]); return -1; }
        if (close(readerPipe[0]) == -1) { perror("Reader - failed to close the reading side of reader pipe"); close(readerPipe[1]); return -1; }

        int inputFile = open(argv[1], O_RDONLY);
        if (inputFile == -1) { perror("Reader - failed to open an input file"); close(readerPipe[1]); return -1; }

        transfer(inputFile, readerPipe[1]);

        if (close(readerPipe[1]) == -1) { perror("Reader - failed to close the writing side of reader pipe"); close(inputFile); return -1; }
        if (close(inputFile) == -1) { perror("Reader - failed to close the input file"); return -1; }
        return 0;
    }

    
    pid_t solver = fork();
    if (solver == -1) { perror("Failed to create the solver process"); kill(reader, SIGINT); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (solver == 0)
    {
        if (close(readerPipe[1]) == -1) { perror("Solver - failed to close the writing side of reader pipe"); close(readerPipe[0]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
        if (close(writerPipe[0]) == -1) { perror("Solver - failed to close the reading side of writer pipe"); close(readerPipe[0]); close(writerPipe[1]); return -1; }
        
        solve(readerPipe[0], writerPipe[1]);

        if (close(readerPipe[0]) == -1) { perror("Solver - failed to close the reading side of reader pipe"); close(writerPipe[1]); return -1; }
        if (close(writerPipe[1]) == -1) { perror("Solver - failed to close the writing side of writer pipe"); return -1; }
        return 0;
    }
    

    pid_t writer = fork();
    if (writer == -1) { perror("Failed to create the writer process"); kill(reader, SIGINT); kill(solver, SIGINT); close(readerPipe[0]); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
    if (writer == 0)
    {
        if (close(readerPipe[0]) == -1) { perror("Writer - failed to close the reading side of reader pipe"); close(readerPipe[1]); close(writerPipe[0]); close(writerPipe[1]); return -1; }
        if (close(readerPipe[1]) == -1) { perror("Writer - failed to close the writing side of reader pipe"); close(writerPipe[0]); close(writerPipe[1]); return -1; }
        if (close(writerPipe[1]) == -1) { perror("Writer - failed to close the writing side of writer pipe"); close(writerPipe[0]); return -1; }

        int outputFile = open(argv[2], O_CREAT | O_WRONLY, 0666);
        if (outputFile == -1) { perror("Writer - failed to open an output file"); close(writerPipe[0]); return -1; }
        if (ftruncate(outputFile, 0) == -1) { perror("Writer - failed to clear the output file"); close(writerPipe[0]); close(outputFile); return -1; }

        transfer(writerPipe[0], outputFile);

        if (close(writerPipe[0]) == -1) { perror("Writer - failed to close the reading side of writer pipe"); close(outputFile); return -1; }
        if (close(outputFile) == -1) { perror("Writer - failed to close the output file"); return -1; }
        return 0;
    }


    if (close(readerPipe[0]) == -1) { perror("Failed to close the reading side of reader pipe"); return -1; }
    if (close(readerPipe[1]) == -1) { perror("Failed to close the writing side of reader pipe"); return -1; }
    if (close(writerPipe[0]) == -1) { perror("Failed to close the reading side of writer pipe"); return -1; }
    if (close(writerPipe[1]) == -1) { perror("Failed to close the writing side of writer pipe"); return -1; }

    waitpid(reader, NULL, 0);
    waitpid(solver, NULL, 0);
    waitpid(writer, NULL, 0);
}