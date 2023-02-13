#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int fd, result;
    size_t size;
    char msg[] = "Hello, reader!";
    int msg_size = sizeof(msg);
    char name[] = "bbb.fifo";

    (void)umask(0);

    mknod(name, S_IFIFO | 0666, 0);

    if ((fd = open(name, O_WRONLY)) < 0)
    {
        printf("Can't open FIFO for writing!");
        exit(-1);
    }
    size = write(fd, msg, msg_size);
    if (size != msg_size)
    {
        printf("Can't write all string to FIFO");
        exit(-1);
    }
    if (close(fd) < 0)
    {
        printf("Writer: can't close FIFO");
    }
    printf("Writer exit!");
    return 0;
}