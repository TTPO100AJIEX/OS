// gcc sh-mem-posix-create.c -o sh-mem-posix-create -lrt
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

// программа создает объект - разделяемую память
int main () {
  char memn[] = "shared-memory"; //  имя объекта
  int shm;
  int mem_size = 100; //размер области

  if ( (shm = shm_open(memn, O_CREAT|O_RDWR, 0666)) == -1 ) {
    printf("Object is already open\n");
    perror("shm_open");
    return 1;
  } else {
    printf("Object is open: name = %s, id = 0x%x\n", memn, shm);
  }
  if (ftruncate(shm, mem_size) == -1) {
    printf("Memory sizing error\n");
    perror("ftruncate");
    return 1;
  } else {
    printf("Memory size set and = %d\n", mem_size);
  }

  char buf[] = "This text is sent to shared memory";
  char *addr;
  addr = mmap(0, mem_size, PROT_WRITE|PROT_READ, MAP_SHARED, shm, 0);
  if (addr == (char*)-1 ) {
    printf("Error getting pointer to shared memory\n");
    perror("mmap");
    return 1;
  }
  memcpy(addr, buf, sizeof(buf));
  printf("Recorded:\n     %s\n", addr);

  close(shm);
  return 0;
}

