#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void readPipe(char* pipeName)
{
    int readDesc = open(pipeName, O_RDONLY); // Open the pipe
    if (readDesc < 0) { printf("Failed to open a pipe %s\n", pipeName); exit(-1); }

    char received[19]; // Create a buffer for the message
    int status = read(readDesc, received, 19); // Read the string to the buffer
    if (status < 0) { printf("Failed to read from pipe %s\n", pipeName); exit(-1); }

    printf("%d - %s\n", getpid(), received); // Print the string from the buffer to the console
    status = close(readDesc); // Close the pipe
    if (status < 0) { printf("Failed to close the pipe %s\n", pipeName); exit(-1); }
}
void writePipe(char* pipeName, char* text)
{
    int writeDesc = open(pipeName, O_WRONLY); // Open the pipe
    if (writeDesc < 0) { printf("Failed to open a pipe %s\n", pipeName); exit(-1); }

    int status = write(writeDesc, text, strlen(text)); // Write the string to the pipe
    if (status < 0) { printf("Failed to write to pipe %s\n", pipeName); exit(-1); }

    status = close(writeDesc); // Close the pipe
    if (status < 0) { printf("Failed to close the pipe %s\n", pipeName); exit(-1); }
}


void worker1(char* readPipeName, char* writePipeName)
{
    // The first process should write its string and then wait for the other process to read and write its string back
    writePipe(writePipeName, "Hello from worker 1"); // Write the string to the writing pipe
    readPipe(readPipeName); // Wait for the data to appear in the reading pipe and read it
}

void worker2(char* readPipeName, char* writePipeName)
{
    // The second process should wait for the first one to write its string, read it and write a string back
    readPipe(readPipeName); // Wait for the data to appear in the reading pipe and read it
    writePipe(writePipeName, "Hello from worker 2"); // Write the string to the writing pipe
}

/*
The program creates two child processes and passes messages from first to second and back
*/
int main(int argc, char** argv)
{
    if (argc < 3) { printf("At least two command line arguments are required\n"); return -1; }

    unlink(argv[1]); unlink(argv[2]); // Delete both pipes to avoid conflicts
    mknod(argv[1], S_IFIFO | 0777, 0); mknod(argv[2], S_IFIFO | 0777, 0); // Create new pipes
    
    int child1 = fork(); // Create the first process
    if (child1 < 0) { printf("Failed to create the first process\n"); return -1; }
    if (child1 == 0) { worker1(argv[1], argv[2]); return 0; } // Do the job of the first process
    
    int child2 = fork(); // Create the second process
    if (child1 < 0) { printf("Failed to create the second process\n"); return -1; }
    if (child2 == 0) { worker2(argv[2], argv[1]); return 0; } // Do the job of the second process

    sleep(1); // To avoid zombie processes and make cleaner output
    return 0;
}