// gcc sh-mem-posix-read.c -o sh-mem-posix-read -lrt
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

//программа открывает объект - общую память
int main () {
  char memn[] = "shared-memory"; //  имя объекта
  char *addr;
  int shm;
  int mem_size = 100; //размер области

  //открыть объект
  if ( (shm = shm_open(memn, O_RDWR, 0666)) == -1 ) {
    printf("Opening error\n");
    perror("shm_open");
    return 1;
  } else {
    printf("Object is open: name = %s, id = 0x%x\n", memn, shm);
  }

  //получить доступ к памяти
  addr = mmap(0, mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
  if (addr == (char*)-1 ) {
    printf("Error getting pointer to shared memory\n");
    return 1;
  }

  //осуществить вывод содержимого разделяемой памяти
  printf("Obtained from shared memory:\n      %s\n", addr);

  //закрыть открытый объект
  close(shm);
  return 0;
}

