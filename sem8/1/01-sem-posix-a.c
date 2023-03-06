// Программа 01-sem-posix-a.c для иллюстрации работы с семафорами
// Эта программа получает доступ к одному posix семафору,
// ждет, пока его значение не станет больше или равным 1
// после запусков программы 01-sem-posix-a.c, а затем уменьшает его на 1
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

int main() {
  char sem_name[] = "/posix-semaphore"; // имя семафора
  sem_t *p_sem;   // адрес семафора

  // Создание семафора
  if((p_sem = sem_open(sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create posix semaphore");
    exit(-1);
  };

  // Обнуленный семафор ожидает, когда его поднимут, чтобы вычесть 1
  for (int i = 0; i < 5; i++)
  {
    if(sem_wait(p_sem) == -1) {
      perror("sem_wait: Incorrect wait of posix semaphore");
      exit(-1);
    };
  }

  // Семафор дождался второго процесса
  printf("Condition is present\n");

  if(sem_close(p_sem) == -1) {
    perror("sem_close: Incorrect close of posix semaphore");
    exit(-1);
  };

  if(sem_unlink(sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of posix semaphore");
    exit(-1);
  };

  return 0;
}

