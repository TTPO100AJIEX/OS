#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "shm/shm.h"
#include "sem/sem.h"
#include "handleVisitor/handleVisitor.h"
#define ROOMS_MEMORY_NAME "hotel_rooms"
#define ROOMS_SEMAPHORE_NAME "hotel_rooms_sync"


int main(int argc, char** argv)
{
    setbuf(stdout, NULL); // Remove the buffering of stdout

    struct Room* rooms = create_memory(ROOMS_MEMORY_NAME, 25 * sizeof(struct Room));
    for (unsigned int i = 0; i < 15; i++) { rooms[i].isForSinglePerson = false; rooms[i].residents.people[0] = 0; rooms[i].residents.people[1] = 0; }
    for (unsigned int i = 15; i < 25; i++) { rooms[i].isForSinglePerson = true; rooms[i].residents.person = 0; }
    sem_t* rooms_sync = create_semaphore(ROOMS_SEMAPHORE_NAME, 1);

    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(5000), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    bind(server, (struct sockaddr *)(&server_address), sizeof(server_address));
    listen(server, 5);

    while (true)
    {
        int client = accept(server, NULL, 0);
        pid_t process = fork();
        if (process == 0) handleVisitor();
        else close(client);
    }

    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && rooms[i].residents.person)
        {
            if (kill(rooms[i].residents.person, SIGINT) == -1) printf("Hotel: failed to kill a visitor\n");
        }
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[0])
        {
            if (kill(rooms[i].residents.people[0], SIGINT) == -1) printf("Hotel: failed to kill a visitor\n");
        }
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[1])
        {
            if (kill(rooms[i].residents.people[1], SIGINT) == -1) printf("Hotel: failed to kill a visitor\n");
        }
    }

    close(server);

    close_semaphore(rooms_sync);
    delete_semaphore(ROOMS_SEMAPHORE_NAME);
    
    delete_memory(ROOMS_MEMORY_NAME);
    return 0;
}