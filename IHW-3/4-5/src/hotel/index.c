#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "shm/shm.h"
#define ROOMS_MEMORY_NAME "hotel_rooms"

#include "sem/sem.h"
#define ROOMS_SEMAPHORE_NAME "hotel_rooms_sync"

struct Room
{
    bool isForSinglePerson;
    /*enum Gender gender;
    union RoomResidents
    {
        int person;
        int people[2];
    } residents;*/
};

int main(int argc, char** argv)
{
    setbuf(stdout, NULL); // Remove the buffering of stdout

    struct Room* rooms = create_memory(ROOMS_MEMORY_NAME, 25 * sizeof(struct Room));
    sem_t* rooms_sync = create_semaphore(ROOMS_SEMAPHORE_NAME, 1);
    
    //for (unsigned int i = 0; i < 15; i++) { rooms[i].isForSinglePerson = false; rooms[i].residents.people[0] = 0; rooms[i].residents.people[1] = 0; }
    //for (unsigned int i = 15; i < 25; i++) { rooms[i].isForSinglePerson = true; rooms[i].gender = NONE; rooms[i].residents.person = 0; }

    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(5000),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    };
    bind(server, (struct sockaddr *)(&server_address), sizeof(server_address));
    listen(server, 5);

    while (true)
    {
        int client = accept(server, NULL, 0);
        pid_t process = fork();
        if (process == 0) handleVisitor();
    }

    while (wait(NULL) != -1) { }
    perror("tesT");

    delete_memory(rooms);
    delete_semaphore(rooms_sync);
    return 0;
}