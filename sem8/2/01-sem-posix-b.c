/* Программа 01-sem-posix-b.c для иллюстрации работы с  семафорами
 * Эта программа получает доступ к одному системному семафору
 * и увеличивает его на 1
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>

int main() {
  const char* shar_object = "/posix-shar-object";
  int shm_id;
  shm_id = shm_open(shar_object, O_RDWR, 0666);
  printf("Objest is open: name = %s, id = 0x%x\n", shar_object, shm_id);

  sem_t* sem_p = (sem_t*)mmap(0, sizeof(sem_t), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);


  // Увеличение значения семафора на 1
  if(sem_post(sem_p) == -1) {
    perror("sem_post: Incorrect post of posix semaphore");
    exit(-1);
  };

  printf("Condition is set\n");

  return 0;
}

