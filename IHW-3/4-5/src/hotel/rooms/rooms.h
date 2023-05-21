#pragma once

#include <stdbool.h>
#include <sys/types.h>
#include "../../protocol.h"
#include "../utils/shm/shm.h"
#include "../utils/sem/sem.h"
#include "../rooms/rooms.h"

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
    pid_t owner; // Owner of the rooms (to determine who must delete everything)
    bool ok; // Has the object been constructed successfully
    
    struct Memory mem; // Shared storage for rooms
    struct Semaphore sem; // Semaphore to synchronize access
    struct Room* storage; // Copy of mem.mem field

    int rooms1; // Amount of one-person rooms
    int rooms2; // Amount of two-person rooms
};

struct Rooms initialize_rooms(const char* memory_name, const char* semaphore_name, int rooms1, int rooms2);
int delete_rooms(struct Rooms* this);

int lock_rooms(struct Rooms* this);
int unlock_rooms(struct Rooms* this);

char* get_rooms_layout(struct Rooms* this);

int take_room(struct Rooms* this, enum Gender gender);
int free_room(struct Rooms* this);