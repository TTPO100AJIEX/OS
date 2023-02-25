#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#define SHM_ID      0x1111    /* ключ разделяемой памяти */
#define PERMS       0666      /* права доступа */

void sys_err (char *msg)
{
  puts (msg);
  exit (1);
}

int main () {
  int shmid;        /* идентификатор разделяемой памяти */
  void *msg_p;      /* адрес сообщения в разделяемой памяти */

  /* получение доступа к сегменту разделяемой памяти */
  if ((shmid = shmget (SHM_ID, getpagesize(), 0)) < 0)
    sys_err ("reader: can not get shared memory segment");

  /* получение адреса сегмента */
  if ((msg_p = shmat (shmid, 0, 0)) == NULL) {
    sys_err ("reader: shared memory attach error");
  }

  // Прочтение данных в разделяемой памяти, сформированных другим процессом
  printf("reader: shared memory using key = %x has read\n", SHM_ID);
  printf("reader: message in shared memory is: %s\n", (char*)msg_p);


  // удаление сегмента разделяемой памяти
  shmdt (msg_p);
  if (shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    sys_err ("eraser: shared memory remove error");
  }
  // Сообщение об удалении сегмента разделяемой памяти
  printf("eraser: shared memory using key = %x deleted\n", SHM_ID);
  return 0;
}

