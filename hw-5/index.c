#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void worker1(char* readPipeName, char* writePipeName)
{
    (void)umask(0);
    char    name[] = "bbb.fifo";
    printf("! %d !", mknod(name, S_IFIFO | 0666, 0));
    int t = open(name, O_RDONLY);
    printf("%d \n", t);
    printf("# %d #", errno);

    mknod(readPipeName, S_IFIFO | 0666, 0);
    mknod(writePipeName, S_IFIFO | 0666, 0);
    int readDesc = open(readPipeName, O_RDONLY);
    int writeDesc = open(writePipeName, O_WRONLY);
    printf("%d %d", readDesc, writeDesc);
    sleep(1);

    char message[] = "Hello from worker 1";
    printf("%d", write(writeDesc, message, sizeof(message)));

    close(readDesc);
    close(writeDesc);
}

void worker2(char* readPipeName, char* writePipeName)
{
    mknod(readPipeName, S_IFIFO | 0666, 0);
    mknod(writePipeName, S_IFIFO | 0666, 0);
    int readDesc = open(readPipeName, O_RDONLY);
    int writeDesc = open(writePipeName, O_WRONLY);
    sleep(1);

    char message[32];
    read(readDesc, message, 32);
    printf("%s", message);
    //char message[] = "Hello from worker 2";
    //write(writeDesc, message, sizeof(message));

    close(readDesc);
    close(writeDesc);
}

int main(int argc, char** argv)
{
    int child1 = fork();
    if (child1 == 0) { worker1(argv[1], argv[2]); return 0; }
    
    int child2 = fork();
    if (child2 == 0) { worker2(argv[2], argv[1]); return 0; }

    sleep(2);
}