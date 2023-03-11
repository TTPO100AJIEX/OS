#include "messages.h"

#ifndef CHUNK_SIZE
    #error "CHUNK_SIZE must be defined"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>

struct msgbuf
{
    long mtype;
    char mtext[CHUNK_SIZE + 1];
};

static bool isUpperCase(char symbol) { return symbol >= 'A' && symbol <= 'Z'; }
static bool isLowerCase(char symbol) { return symbol >= 'a' && symbol <= 'z'; }
static bool isLetter(char symbol) { return isUpperCase(symbol) || isLowerCase(symbol); }

void solve(const int input, const int output)
{
    struct msgbuf buffer = { 1 };
    char* wordStart;
    bool isFirstWord = true;
    enum Status { WORD, SKIP, CHECK } status = CHECK;
    
    int lastRead = -1;
    while (lastRead != 1)
    {
        lastRead = msgrcv(input, &buffer, CHUNK_SIZE + 1, 1, 0);
        if (lastRead == -1) { perror("Solve - read failed"); return; }
       
        for (int i = 1; i < lastRead; i++)
        {
            switch (status)
            {
                case WORD:
                {
                    if (isLetter(buffer.mtext[i])) break;
                    const unsigned int wordLength = &buffer.mtext[i] - wordStart;
                    if (wordLength != 0)
                    {
                        memmove(buffer.mtext, wordStart, wordLength);
                        if (msgsnd(output, &buffer, wordLength, 0) == -1) { perror("Solve - write failed"); return; }
                    }
                    status = CHECK;
                    break;
                }
                case SKIP:
                {
                    if (!isLetter(buffer.mtext[i])) status = CHECK;
                    break;
                }
                case CHECK:
                {
                    if (isUpperCase(buffer.mtext[i]))
                    {
                        status = WORD;
                        wordStart = &buffer.mtext[i];
                        i--;
                        if (!isFirstWord) { buffer.mtext[i] = ' '; wordStart--; }
                        else isFirstWord = false;
                        break;
                    }
                    if (isLowerCase(buffer.mtext[i])) status = SKIP;
                    break;
                }
            }
        }
        
        if (status == WORD)
        {
            const unsigned int wordLength = &buffer.mtext[lastRead] - wordStart;
            if (wordLength != 0)
            {
                memmove(buffer.mtext, wordStart, wordLength);
                if (msgsnd(output, &buffer, wordLength, 0) == -1) { perror("Solve - write failed"); return; }
                wordStart = &buffer.mtext[1];
            }
        }
    }

    if (msgsnd(output, &buffer, 0, 0) == -1) { perror("Solve - write failed"); return; }
}