#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#define CHUNK_SIZE 5000

bool isUpperCase(char symbol) { return symbol >= 'A' && symbol <= 'Z'; }
bool isLowerCase(char symbol) { return symbol >= 'a' && symbol <= 'z'; }
bool isLetter(char symbol) { return isUpperCase(symbol) || isLowerCase(symbol); }

void solve(int input, int output)
{
    char inbuffer[CHUNK_SIZE + 1];
    char* wordStart = NULL;
    bool isFirstWord = true;
    enum Status { WORD, SKIP, CHECK } status = CHECK;
    
    int lastRead = -1;
    while (lastRead != 0)
    {
        lastRead = read(input, inbuffer + sizeof(char), CHUNK_SIZE);
        if (lastRead == -1) { perror("Solver - failed to read the input from the reader pipe"); return; }

        for (unsigned int i = 1; i <= lastRead; i++)
        {
            switch (status)
            {
                case WORD:
                {
                    if (isLetter(inbuffer[i])) break;
                    const unsigned int wordLength = &inbuffer[i] - wordStart;
                    if (write(output, wordStart, wordLength) != wordLength) { perror("Solver - failed to write to the writer pipe"); return; }
                    status = CHECK;
                    break;
                }
                case SKIP:
                {
                    if (!isLetter(inbuffer[i])) status = CHECK;
                    break;
                }
                case CHECK:
                {
                    if (isUpperCase(inbuffer[i]))
                    {
                        status = WORD;
                        wordStart = &inbuffer[i];
                        i--;
                        if (!isFirstWord) { inbuffer[i] = ' '; wordStart -= sizeof(char); }
                        else isFirstWord = false;
                        break;
                    }
                    if (isLowerCase(inbuffer[i])) status = SKIP;
                    break;
                }
            }
        }
        if (status == WORD)
        {
            const unsigned int wordLength = &inbuffer[lastRead + 1] - wordStart;
            if (write(output, wordStart, wordLength) != wordLength) { perror("Solver - failed to write to the writer pipe"); return; }
            wordStart = &inbuffer[1];
        }
    }
}

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