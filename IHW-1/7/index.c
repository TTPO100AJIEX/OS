#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "pipes.h"

#define READER_PIPE_NAME "reader.fifo"
#define WRITER_PIPE_NAME "writer.fifo"

int main(int argc, char** argv)
{
    if (argc < 3) { printf("Not enough command line arguments specified"); return -1; }

    if (mkfifo(READER_PIPE_NAME, 0666) == -1) { perror("Failed to create a reader pipe"); return -1; }
    if (mkfifo(WRITER_PIPE_NAME, 0666) == -1) { perror("Failed to create a writer pipe"); unlink(READER_PIPE_NAME); return -1; }


    pid_t io = fork();
    if (io == -1) { perror("Failed to create the io process"); unlink(READER_PIPE_NAME); unlink(WRITER_PIPE_NAME); return -1; }
    if (io == 0)
    {
        pid_t writer = fork();
        if (writer == -1) { perror("IO - failed to create the writer process"); return -1; }
        if (writer == 0)
        {
            int outputFile = open(argv[2], O_CREAT | O_WRONLY, 0666);
            if (outputFile == -1) { perror("Writer - failed to open an output file"); return -1; }
            if (ftruncate(outputFile, 0) == -1) { perror("Writer - failed to clear the output file"); close(outputFile); return -1; }

            int inputPipe = open(WRITER_PIPE_NAME, O_RDONLY);
            if (inputPipe == -1) { perror("Writer - failed to open the writer pipe for reading"); close(outputFile); return -1; }

            transfer(inputPipe, outputFile);

            if (close(outputFile) == -1) { perror("Writer - failed to close the output file"); close(inputPipe); return -1; }
            if (close(inputPipe) == -1) { perror("Writer - failed to close the writer pipe"); return -1; }
            return 0;
        }

        int inputFile = open(argv[1], O_RDONLY);
        if (inputFile == -1) { perror("IO - failed to open an input file"); kill(writer, SIGINT); return -1; }

        int outputPipe = open(READER_PIPE_NAME, O_WRONLY);
        if (outputPipe == -1) { perror("IO - failed to open the reader pipe for writing"); kill(writer, SIGINT); close(inputFile); return -1; }

        transfer(inputFile, outputPipe);

        if (close(inputFile) == -1) { perror("IO - failed to close the input file"); kill(writer, SIGINT); close(outputPipe); return -1; }
        if (close(outputPipe) == -1) { perror("IO - failed to close the reader pipe"); kill(writer, SIGINT); return -1; }
        
        waitpid(writer, NULL, 0);

        return 0;
    }

    
    pid_t solver = fork();
    if (solver == -1) { perror("Failed to create the solver process"); kill(io, SIGINT); unlink(READER_PIPE_NAME); unlink(WRITER_PIPE_NAME); return -1; }
    if (solver == 0)
    {
        int inputPipe = open(READER_PIPE_NAME, O_RDONLY);
        if (inputPipe == -1) { perror("Solver - failed to open the reader pipe for reading"); return -1; }

        int outputPipe = open(WRITER_PIPE_NAME, O_WRONLY);
        if (outputPipe == -1) { perror("Solver - failed to open the writer pipe for writing"); close(inputPipe); return -1; }
        
        solve(inputPipe, outputPipe);

        if (close(inputPipe) == -1) { perror("Solver - failed to close the reader pipe"); close(outputPipe); return -1; }
        if (close(outputPipe) == -1) { perror("Solver - failed to close the writer pipe"); return -1; }
        return 0;
    }


    waitpid(io, NULL, 0);
    waitpid(solver, NULL, 0);

    if (unlink(READER_PIPE_NAME) == -1) { perror("Failed to delete the reader pipe"); unlink(WRITER_PIPE_NAME); return -1; }
    if (unlink(WRITER_PIPE_NAME) == -1) { perror("Failed to delete the writer pipe"); return -1; }
}