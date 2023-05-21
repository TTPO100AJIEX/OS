#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

const int alphabet_size = 'z' - 'a' + 1;
int solve(int link_number)
{
    // Construct the names of the files
    char prev_name[] = "a\0";
    char cur_name[] = "aa";
    if (link_number != 0)
    {
        prev_name[0] = 'a' + ((link_number - 1) / alphabet_size);
        prev_name[1] = 'a' + ((link_number - 1) % alphabet_size);
        
        cur_name[0] = 'a' + (link_number / alphabet_size);
        cur_name[1] = 'a' + (link_number % alphabet_size);
    }
    
    // Construct the full name of the new file
    char folder[16] = "folder/";
    strcat(folder, cur_name);
    
    // Create the symlink
    if (symlink(prev_name, folder) == -1) { perror("Failed to create a symlimk"); return 0; }
    
    // Open the file via symlink
    int fd = open(folder, O_RDWR), answer = 0;
    if (fd != -1) // If it opened correctly
    {
        if (close(fd) == -1) perror("Failed to close the file"); // Close the file
        answer = solve(link_number + 1) + 1; // Calculate the answer recursively
    }

    if (unlink(folder) == -1) perror("Failed to remove the link"); // Delete the symlink
    return answer;
}


int main(void)
{
    // Create the folder
    if (mkdir("folder", 0777) == -1)
    {
        perror("Failed to create a folder");
        return 1;
    }
    // Create the file
    if (creat("folder/a", 0777) == -1)
    {
        perror("Failed to create a file");
        return 1;
    }

    // Print the answer
    printf("%d", solve(0) + 1);
    
    // Delete the file
    if (unlink("folder/a") == -1)
    {
        perror("Failed to remove the file");
        return 1;
    }
    // Delete the folder
    if (rmdir("folder") == -1)
    {
        perror("Failed to remove the folder");
        return 1;
    }

    return 0;
}