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
    char inbuffer[CHUNK_SIZE + 1]; // Buffer for data. The first symbol is reserved for putting a space character if needed.
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
    while (lastRead != 0) // While there is data to process
    {
        lastRead = read(input, inbuffer + 1, CHUNK_SIZE); // Read data
        if (lastRead == -1) { perror("Solve - read failed!"); return; } // Print an error if a read failed

        for (int i = 1; i <= lastRead; i++) // Loop through all symbols
        {
            switch (status)
            {
                case WORD:
                {
                    if (isLetter(inbuffer[i])) break; // If the current symbol is a letter, the word continues
                    // The current symbol is not a letter, so a word has been found and needs to be sent to the writer
                    const unsigned int wordLength = &inbuffer[i] - wordStart; // Calculate its length
                    if (wordLength != 0)
                    {
                        // wordLength may be 0 if a chunk ended with the end of the word
                        // It is undefined behavior to write 0 bytes into a fifo, so the "if" is required
                        if (write(output, wordStart, wordLength) != wordLength) { perror("Solver - write failed!"); return; } // Write the word if it is not empty.
                    }
                    status = CHECK; // Now the algorithm needs to start looking for a next word
                    break;
                }
                case SKIP:
                {
                    if (!isLetter(inbuffer[i])) status = CHECK; // If the current symbol is not letter, the word has ended and the algorithm needs to start looking for a next word
                    break;
                }
                case CHECK:
                {
                    if (isUpperCase(inbuffer[i]))
                    {
                        // A beginning of the word is found, and this word should be included in the answer
                        status = WORD;
                        wordStart = &inbuffer[i]; // Save the pointer to the first symbol of this word
                        if (!isFirstWord) { inbuffer[i - 1] = ' '; wordStart--; } // Add space before the word if needed. Overflow is not possible because buffer[0] is specifically reserved for space
                        else isFirstWord = false; // If it was the first word, all next words are not first
                        break;
                    }
                    if (isLowerCase(inbuffer[i])) status = SKIP; // A beginning of the word is found, but this word should not be included in the answer
                    break;
                }
            }
        }
        
        if (status == WORD)
        {
            // If the buffer ended but a word has not been completely found, write a part of the word that was found
            const unsigned int wordLength = &inbuffer[lastRead] - wordStart + 1;
            if (write(output, wordStart, wordLength) != wordLength) { perror("Solver - write failed!"); return; }
            wordStart = &inbuffer[1]; // The beginning of the next buffer continues the current word
        }
    }
}