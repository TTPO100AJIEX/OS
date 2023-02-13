#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int buf_size = 25;
const int mes_size = 14;

int main()
{
    int fd[2], result;
    ssize_t size;
    char str_buf[buf_size];

    if (pipe(fd) < 0)
    {
        printf("Can't open pipe!");
        exit(-1);
    }

    result = fork();
    if (result < 0)
    {
        printf("Can't fork child!");
        exit(-1);
    }
    else if (result > 0)
    {
        if (close(fd[0]) < 0)
        {
            printf("Can't close reading side of pipe!");
            exit(-1);
        }
        size = write(fd[1], "Hello, world!", mes_size);
        if (size != mes_size)
        {
            printf("Can't write all string to pipe!");
            exit(-1);
        }
        if (close(fd[1]) < 0)
        {
            printf("parent: Can't close writing side of pipe!");
            exit(-1);
        }
        printf("Parent exit!");
    }
    else
    {
        if (close(fd[1]) < 0)
        {
            printf("Can't close writing side of pipe!");
            exit(-1);
        }
        size = read(fd[0], str_buf, mes_size);
        if (size < 0)
        {
            printf("Can't write all string to pipe!");
            exit(-1);
        }
        printf("Child exit, std_buf: %s", str_buf);
        if (close(fd[0]) < 0)
        {
            printf("Can't close reading side of pipe!");
            exit(-1);
        }
    }

    return 0;
}