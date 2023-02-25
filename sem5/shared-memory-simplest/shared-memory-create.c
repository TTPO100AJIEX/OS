#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>

//#define SHM_ID      0x1111    // ключ разделяемой памяти
#define PERMS       0666      // права доступа

void sys_err (char *msg) {
  puts (msg);
  exit (1);
}

int main () {
  int shmid;        // идентификатор разделяемой памяти
  key_t SHM_ID = ftok("./shared-memory-simplest", 1);

  /* создание сегмента разделяемой памяти */
  int shar_mem_size = getpagesize();
  if ((shmid = shmget (SHM_ID, shar_mem_size, PERMS | IPC_CREAT)) < 0) {
    sys_err ("creator: can not create shared memory segment");
  }
  // Сообщение о создании сегмента разделяемой памяти
  printf("creator: shared memory using key = %x created\n", SHM_ID);
  printf("creator: size of shared memory = %d\n", shar_mem_size);

  return 0;
}

