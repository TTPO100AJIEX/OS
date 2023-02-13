#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int buf_size = 25;
const int mes_size = 14;

int main()
{
    int fd[2];
    ssize_t size;
    char string[] = "Hello, world!";
    char str_buf[buf_size];

    strcpy(str_buf, "@@@@@@@@@@@@@@@@@@@@@@@");

    if (pipe(fd) < 0)
    {
        printf("Can't open pipe!");
        exit(-1);
    }

    size = write(fd[1], string, mes_size);

    if (size < 0)
    {
        printf("Can't write all string to pipe!");
        exit(-1);
    }

    size = read(fd[0], str_buf, mes_size);

    if (size < 0)
    {
        printf("Can't read string from pipe!");
        exit(-1);
    }

    printf("%s", str_buf);

    if (close(fd[0]) < 0)
    {
        printf("Can't close reading side of pipe!");
        exit(-1);
    }
    if (close(fd[1]) < 0)
    {
        printf("Can't close writing side of pipe!");
        exit(-1);
    }

    return 0;
}