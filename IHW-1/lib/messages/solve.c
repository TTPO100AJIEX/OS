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
    char mtext[CHUNK_SIZE + 1]; // Buffer for data. The first symbol is reserved for the algorithm
};

static bool isUpperCase(char symbol) { return symbol >= 'A' && symbol <= 'Z'; }
static bool isLowerCase(char symbol) { return symbol >= 'a' && symbol <= 'z'; }
static bool isLetter(char symbol) { return isUpperCase(symbol) || isLowerCase(symbol); }

void solve(const int input, const int output)
{
    struct msgbuf buffer = { 1, { 0 } };
    char* wordStart = NULL; // Pointer to the beginning of the word that is being processed
    bool isFirstWord = true; // Flag to put spaces between words correctly to avoid the space in the beginning or in the end
    /*
        Current state of the algorithm:
            collecting the symbols of a word (if there was an uppercase letter after a delimiter),
            skipping a word (if there was a lowercase letter after a delimiter),
            checking whether the current symbol starts a new word (to properly process multiple delimiters in a row)
    */
    enum Status { WORD, SKIP, CHECK } status = CHECK;
    
    int lastRead = -1;
    while (lastRead != 1) // While there is data to process
    {
        lastRead = msgrcv(input, &buffer, CHUNK_SIZE + 1, 1, 0); // Get data from the message queue
        if (lastRead == -1) { perror("Solve - read failed"); return; } // Print an error if a read failed
       
        for (int i = 1; i < lastRead; i++) // Loop through all symbols
        {
            switch (status)
            {
                case WORD:
                {
                    if (isLetter(buffer.mtext[i])) break; // If the current symbol is a letter, the word continues
                    // The current symbol is not a letter, so a word has been found and needs to be sent to the writer
                    const unsigned int wordLength = &buffer.mtext[i] - wordStart; // Calculate its length
                    if (wordLength != 0)
                    {
                        // wordLength may be 0 if a chunk ended with the end of the word
                        // We do not want to send an empty message until all data is processed, so the "if" is required
                        memmove(buffer.mtext, wordStart, wordLength); // Copy the word to the beginning of the buffer
                        if (msgsnd(output, &buffer, wordLength, 0) == -1) { perror("Solve - write failed"); return; } // Put a word into the message queue
                    }
                    status = CHECK; // Now the algorithm needs to start looking for a next word
                    break;
                }
                case SKIP:
                {
                    if (!isLetter(buffer.mtext[i])) status = CHECK; // If the current symbol is not letter, the word has ended and the algorithm needs to start looking for a next word
                    break;
                }
                case CHECK:
                {
                    if (isUpperCase(buffer.mtext[i]))
                    {
                        // A beginning of the word is found, and this word should be included in the answer
                        status = WORD;
                        wordStart = &buffer.mtext[i]; // Save the pointer to the first symbol of this word
                        if (!isFirstWord) { buffer.mtext[i - 1] = ' '; wordStart--; } // Add space before the word if needed. Overflow is not possible because buffer[0] is specifically reserved for space
                        else isFirstWord = false; // If it was the first word, all next words are not first
                        break;
                    }
                    if (isLowerCase(buffer.mtext[i])) status = SKIP; // A beginning of the word is found, but this word should not be included in the answer
                    break;
                }
            }
        }
        
        if (status == WORD)
        {
            // If the buffer ended but a word has not been completely found, send a part of the word that was found
            const unsigned int wordLength = &buffer.mtext[lastRead] - wordStart;
            memmove(buffer.mtext, wordStart, wordLength); // Copy the word to the beginning of the buffer
            if (msgsnd(output, &buffer, wordLength, 0) == -1) { perror("Solve - write failed"); return; }
            wordStart = &buffer.mtext[1]; // The beginning of the next buffer continues the current word
        }
    }

    if (msgsnd(output, &buffer, 0, 0) == -1) { perror("Solve - write failed"); return; } // Send an empty message to tell the other process that all data has been processed
}