#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "pipes.h"

#define READER_PIPE_NAME "reader.fifo"
#define WRITER_PIPE_NAME "writer.fifo"

int main(int argc, char** argv)
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