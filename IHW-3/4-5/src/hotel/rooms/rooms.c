#include "rooms.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

struct Rooms initialize_rooms(const char* memory_name, const char* semaphore_name)
{
    struct Rooms rooms = { .ok = false };
    if (!(rooms.memory_name = malloc(strlen(memory_name) + 1))) return rooms;
    if (!(rooms.rooms = create_memory(memory_name, 25 * sizeof(struct Room)))) return rooms;
    if (!(rooms.semaphore_name = malloc(strlen(semaphore_name) + 1))) return rooms;
    if (!(rooms.rooms_sync = create_semaphore(semaphore_name, 1))) return rooms;

    strcpy(rooms.memory_name, memory_name);
    strcpy(rooms.semaphore_name, semaphore_name);
    for (unsigned int i = 0; i < 15; i++) { rooms.rooms[i].isForSinglePerson = false; rooms.rooms[i].residents.people[0] = 0; rooms.rooms[i].residents.people[1] = 0; }
    for (unsigned int i = 15; i < 25; i++) { rooms.rooms[i].isForSinglePerson = true; rooms.rooms[i].residents.person = 0; }
    rooms.ok = true;
    return rooms;
}
int close_rooms(struct Rooms* rooms)
{
    return close_semaphore(rooms->rooms_sync);
}
int destroy_rooms(struct Rooms* rooms)
{
    int err = 0;
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms->rooms[i].isForSinglePerson && rooms->rooms[i].residents.person)
        {
            if (kill(rooms->rooms[i].residents.person, SIGINT) == -1) err = errno;
        }
        if (!rooms->rooms[i].isForSinglePerson && rooms->rooms[i].residents.people[0])
        {
            if (kill(rooms->rooms[i].residents.people[0], SIGINT) == -1) err = errno;
        }
        if (!rooms->rooms[i].isForSinglePerson && rooms->rooms[i].residents.people[1])
        {
            if (kill(rooms->rooms[i].residents.people[1], SIGINT) == -1) err = errno;
        }
    }
    if (close_rooms(rooms) == -1) err = errno;
    if (delete_semaphore(rooms->semaphore_name) == -1) err = errno;
    if (delete_memory(rooms->memory_name) == -1) err = errno;
    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}