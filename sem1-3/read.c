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

    if ((fd = open("read.c", O_RDONLY, 0666)) < 0)
    {
        printf("Can\'t open file\n");
        exit(-1);
    }

    char* buf = malloc(2500);
    printf("%d", (int)(read(fd, buf, 2500)));
    printf("%s", buf);
}