#include "pipes.h"

#ifndef CHUNK_SIZE
    #error "CHUNK_SIZE must be defined"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

static bool isUpperCase(char symbol) { return symbol >= 'A' && symbol <= 'Z'; }
static bool isLowerCase(char symbol) { return symbol >= 'a' && symbol <= 'z'; }
static bool isLetter(char symbol) { return isUpperCase(symbol) || isLowerCase(symbol); }

void solve(const int input, const int output)
{
    char inbuffer[CHUNK_SIZE + 1];
    char* wordStart;
    bool isFirstWord = true;
    enum Status { WORD, SKIP, CHECK } status = CHECK;
    
    int lastRead = -1;
    while (lastRead != 0)
    {
        lastRead = read(input, inbuffer + 1, CHUNK_SIZE);
        if (lastRead == -1) { perror("Solve - read failed!"); return; }

        for (int i = 1; i <= lastRead; i++)
        {
            switch (status)
            {
                case WORD:
                {
                    if (isLetter(inbuffer[i])) break;
                    const unsigned int wordLength = &inbuffer[i] - wordStart;
                    if (wordLength != 0)
                    {
                        if (write(output, wordStart, wordLength) != wordLength) { perror("Solver - write failed!"); return; }
                    }
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
                        if (!isFirstWord) { inbuffer[i] = ' '; wordStart--; }
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
            if (write(output, wordStart, wordLength) != wordLength) { perror("Solver - write failed!"); return; }
            wordStart = &inbuffer[1];
        }
    }
}