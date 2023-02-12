/*
Run the program:
gcc index.c -o a.exe
./a.exe a.exe test.out
*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUF_SIZE 1024 // Size of chunks to process the file

int main(int argc, char** argv)
{
    int fdIn = open(argv[1], O_RDONLY); // Open the input file
    if (fdIn < 0) { printf("Can\'t open file!"); return -1; } // If the file failed to open, print the error and stop

    struct stat infileStats; fstat(fdIn, &infileStats); // Get some information about the file
    unsigned int perms =
        (infileStats.st_mode & S_IRUSR) + (infileStats.st_mode & S_IWUSR) + (infileStats.st_mode & S_IXUSR) + // Extract permissions for owner
        (infileStats.st_mode & S_IRGRP) + (infileStats.st_mode & S_IWGRP) + (infileStats.st_mode & S_IXGRP) + // Exract permission for group
        (infileStats.st_mode & S_IROTH) + (infileStats.st_mode & S_IWOTH) + (infileStats.st_mode & S_IXOTH); // Extract permissions for everyone else
    umask(0); // Remove the mask for permissions

    unlink(argv[2]); // Delete the old file if it exists (required for permissions to change during open)
    int fdOut = open(argv[2], O_WRONLY | O_CREAT, perms); // Create and open the output file
    unsigned int lastSize;
    char buffer[BUF_SIZE]; // Initialize the buffer
    while ((lastSize = read(fdIn, buffer, BUF_SIZE))) // While read() did read something from the input file
        write(fdOut, buffer, lastSize); // Write it to the output file
    close(fdIn); close(fdOut); // Close both files
    return 0;
}