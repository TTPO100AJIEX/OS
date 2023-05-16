#pragma once

#include "../sem/sem.h"
#include "../shm/shm.h"
#include "../../protocol.h"

#include <stdbool.h>
#include <sys/types.h>

struct Room
{
    enum Gender gender;
    union RoomResidents
    {
        pid_t person;
        pid_t people[2];
    } residents;
};
struct Rooms
{
    char* memory_name;
    struct Room* rooms;

    char* semaphore_name;
    sem_t* rooms_sync;

    bool ok;
};

struct Rooms initialize_rooms(const char* memory_name, const char* semaphore_name);
int close_rooms(struct Rooms* rooms);
int destroy_rooms(struct Rooms* rooms);

int take_room(struct Rooms* rooms, enum Gender gender);
int free_room(struct Rooms* rooms);