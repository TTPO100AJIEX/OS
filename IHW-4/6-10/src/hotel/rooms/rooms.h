#pragma once

#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../../protocol.h"

struct Room
{
    size_t id;
    enum Gender gender;
    union RoomResidents
    {
        struct Resident
        {
            size_t id;
            struct timeval leave_time;
            struct sockaddr_in client;
        } person, people[2];
    } residents;
};

void init_rooms();
void destroy_rooms();

void print_rooms();

const struct Room* take_room(size_t id, enum Gender gender, unsigned int stay_time, struct sockaddr_in client);
const struct Room* free_room(size_t room_id, size_t visitor_id);