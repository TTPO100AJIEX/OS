// gcc sh-mem-posix-delete.c -o sh-mem-posix-delete -lrt
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

// программа удаляет объект - общая память
int main () {
  char * memn = "shared-memory"; // имя объекта
  if(shm_unlink(memn) == -1) {
    printf("Shared memory is absent\n");
    perror("shm_unlink");
    return 1;
  }
  return 0;
}

