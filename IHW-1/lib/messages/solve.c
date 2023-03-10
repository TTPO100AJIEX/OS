#include "messages.h"

#ifndef CHUNK_SIZE
    #error "CHUNK_SIZE must be defined"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

struct msgbuf
{
    long mtype;
    //char mtext[CHUNK_SIZE + 1];
    char* mtext;
};

static bool isUpperCase(char symbol) { return symbol >= 'A' && symbol <= 'Z'; }
static bool isLowerCase(char symbol) { return symbol >= 'a' && symbol <= 'z'; }
static bool isLetter(char symbol) { return isUpperCase(symbol) || isLowerCase(symbol); }

void solve(const int input, const int output)
{
    char inbuffer[CHUNK_SIZE + 1];
    struct msgbuf buffer = { 1 };
    buffer.mtext = malloc(CHUNK_SIZE);
    char* wordStart;
    bool isFirstWord = true;
    enum Status { WORD, SKIP, CHECK } status = CHECK;
    
    int lastRead = -1;
    while (lastRead != 0)
    {
        lastRead = msgrcv(input, &buffer, CHUNK_SIZE, 1, 0);
        if (lastRead == -1) { perror("msgrcv"); return; }
        memcpy(inbuffer + 1, buffer.mtext, lastRead);
       
        for (int i = 1; i <= lastRead; i++)
        {
            switch (status)
            {
                case WORD:
                {
                    if (isLetter(inbuffer[i])) break;
                    const unsigned int wordLength = &inbuffer[i] - wordStart;
                    memcpy(buffer.mtext, wordStart, wordLength);
                    if (wordLength != 0) msgsnd(output, &buffer, wordLength, 0);
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
            memcpy(buffer.mtext, wordStart, wordLength);
            if (wordLength != 0) msgsnd(output, &buffer, wordLength, 0);
            wordStart = &inbuffer[1];
        }
    }

    msgsnd(output, &buffer, 0, 0);
}