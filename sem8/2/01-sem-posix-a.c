// Программа 01-sem-posix-a.c для иллюстрации работы с семафорами
// Эта программа получает доступ к одному posix семафору,
// ждет, пока его значение не станет больше или равным 1
// после запусков программы 01-sem-posix-a.c, а затем уменьшает его на 1
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
  const char* shar_object = "/posix-shar-object";
  int shm_id;
  shm_id = shm_open(shar_object, O_CREAT | O_RDWR, 0666);
  printf("Objest is open: name = %s, id = 0x%x\n", shar_object, shm_id);
  ftruncate(shm_id, sizeof(sem_t));
  sem_t* sem_p = (sem_t*)mmap(0, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);
  if ((sem_init(sem_p, 1, 0) == -1))
  {
    perror("sem_init");
    exit(-1);
  }

  sem_wait(sem_p);

  printf("Condition is present\n");

  if(sem_close(sem_p) == -1) {
    perror("sem_close: Incorrect close of posix semaphore");
    exit(-1);
  };
  shm_unlink(shar_object);

  return 0;
}

