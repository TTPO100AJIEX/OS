#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int fd;
    size_t size;
    int buf_size = 20;
    char str_buf[buf_size];
    char name[] = "bbb.fifo";
    if ((fd = open(name, O_RDONLY)) < 0)
    {
        printf("Can't open FIFO for reading!");
        exit(-1);
    }
    size = read(fd, str_buf, buf_size);

    if (size < 0)
    {
        printf("Can't read string from pipe!");
        exit(-1);
    }

    printf("%s", str_buf);
    if (close(fd) == 0)
    {
        printf("Reader: can't close FIFO");
    }
    printf("Reader exit!");
    return 0;
}