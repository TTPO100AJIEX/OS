#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

const int len = 13;
int main(void)
{
    int fd;
    size_t size;
    char string[] = "Hello, world!";

    (void)umask(0);

    if ((fd = open("01-myfile-file-syscall.txt", O_WRONLY | O_CREAT, 0666)) < 0)
    {
        printf("Can\'t open file\n");
        exit(-1);
    }

    size = write(fd, string, len);

    if (size != len)
    {
        printf("Can\'t write all string\n");
        exit(-1);
    }
    if (close(fd) < 0)
    {
        printf("Can\'t close file\n");

        return 0;
    }
}