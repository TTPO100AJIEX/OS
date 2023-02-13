#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int buf_size = 25;
const int mes_size = 14;

int main()
{
    int fd1[2], fd2[2], result;
    pipe(fd1); pipe(fd2);
    char str_buf[buf_size];

    result = fork();
    if (result > 0)
    {
        // parent
        write(fd1[1], "Hello, Pworld", mes_size);
        read(fd2[0], str_buf, mes_size);
        printf("Parent exit, str_buf: %s\n", str_buf);
    }
    else
    {
        // child
        read(fd1[0], str_buf, mes_size);
        write(fd2[1], "Hello, Cworld", mes_size);
        printf("Child exit, str_buf: %s\n", str_buf);
    }

    return 0;
}