#pragma once

#include <sys/types.h>
#include "../../protocol.h"

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
    pid_t owner;
    struct Room* rooms;
    int rooms1;
    int rooms2;
};

struct Rooms initialize_rooms(struct Room* rooms_storage, int rooms1, int rooms2);
int delete_rooms(struct Rooms* rooms);

char* get_rooms_layout(struct Rooms* rooms);

int take_room(struct Rooms* rooms, enum Gender gender);
int free_room(struct Rooms* rooms);