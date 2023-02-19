#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main() {
    char* file="pipe.txt";
    int state;
    unlink(file);
    state = mknod(file, S_IFIFO | 0777, 0);
    printf("state %d\n", state);
    return 0;
}