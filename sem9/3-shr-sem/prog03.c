#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <stdio.h>
#include <stdlib.h>

int main() {
    const int array_size = 4;
    const int pID = 2;
    key_t shm_key;
    int shmid;
    int*  count_array;
    char pathname[]="../3-shr-sem";
    shm_key = ftok(pathname, 0);

    if((shmid = shmget(shm_key, sizeof(int)*array_size,
                       0666 | IPC_CREAT | IPC_EXCL)) < 0)  {
        if((shmid = shmget(shm_key, sizeof(int)*array_size, 0)) < 0) {
            printf("Can\'t connect to shared memory\n");
            exit(-1);
        };
        count_array = (int*)shmat(shmid, NULL, 0);
        printf("Connect to Shared Memory\n");
    } else {
        count_array = (int*)shmat(shmid, NULL, 0);
        for(int i = 0; i < array_size; ++i) {
            count_array[i] = 0;
        }
        printf("New Shared Memory\n");
    }

    struct sembuf mybuf;
    mybuf.sem_num = 0;
    mybuf.sem_flg = 0;
    int   semid;
    key_t sem_key = shm_key;
    if((semid = semget(sem_key, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0){
        if((semid = semget(sem_key, 1, 0)) < 0) {
            printf("Can\'t connect to semaphor\n");
            exit(-1);
        };
        printf("Connect to Semaphor\n");
    } else {
        semctl(semid, 0, SETVAL, 1);
        printf("New Semaphor\n");
    }

    int sum = 0;

    mybuf.sem_op  = -1;
    semop(semid, &mybuf, 1);

    ++count_array[pID];
    for(int i = 0; i < array_size-1; ++i) {
        sum += count_array[i];
    }
    // delay
    for(unsigned long int i = 0; i < 0xFFFFFFFFL; ++i);
    count_array[array_size-1] = sum;

    printf("C1: %d; C2: %d; C3: %d; SUM: %d\n",
           count_array[0], count_array[1],
           count_array[2], count_array[3]);

    mybuf.sem_op  = 1;
    semop(semid, &mybuf, 1);

    shmdt(count_array);
    return 0;
}
