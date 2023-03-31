// Заголовочный файл, содержащий общие данные для писателей и читателей
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

#define BUF_SIZE 10     // Размер буфера ячеек
// Структура для хранения в разделяемой памяти буфера и необходимых данных
// Задает кольцевой буфер, доступный процессам
typedef struct {
  int store[BUF_SIZE];  // буфер для заполнения ячеек
  int readers;      // индикатор количества читателей
  int first_reader;       // идентификатор процесса читателя
  int second_reader;       // идентификатор процесса читателя
  // int have_writer;      // индикатор наличия писателя
  int writer_pid;       // идентификатор процесса писателя

  int readerI;
  int writerI;
} shared_memory;

// имя области разделяемой памяти
extern const char* shar_object ;
extern int buf_id;        // дескриптор объекта памяти
extern shared_memory *buffer;    // указатель на разделямую память, хранящую буфер

// имя семафора для занятых ячеек
extern const char *full_sem_name;
extern sem_t *full;   // указатель на семафор занятых ячеек

// имя семафора для свободных ячеек
extern const char *empty_sem_name;
extern sem_t *empty;   // указатель на семафор свободных ячеек

// имя семафора (мьютекса) для критической секции,
// обеспечивающей доступ к буферу
extern const char *writeMutex_sem_name;
extern sem_t *writeMutex;   // указатель на семафор читателей
extern const char *readMutex_sem_name;
extern sem_t *readMutex;   // указатель на семафор читателей


// Имя семафора для управления доступом.
// Позволяет читателю дождаться разрешения от читателя.
// Даже если читатель стартовал первым
extern const char *admin_sem_name;
extern sem_t *admin;   // указатель на семафор читателей

// Функция осуществляющая при запуске общие манипуляции с памятью и семафорами
// для децентрализованно подключаемых процессов читателей и писателей.
void init(void);

// Функция закрывающая семафоры общие для писателей и читателей
void close_common_semaphores(void);

// Функция, удаляющая все семафоры и разделяемую память
void unlink_all(void);
