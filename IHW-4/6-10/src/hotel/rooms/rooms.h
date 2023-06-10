#pragma once

#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../../protocol.h"

struct Room
{
    enum Gender gender;
    union RoomResidents
    {
        struct Resident
        {
            struct timeval leave_time;
            in_addr_t ip;
            in_port_t port;
        } person, people[2];
    } residents;
};

void init_rooms();
void destroy_rooms();

void print_rooms();

int take_room(enum Gender gender, unsigned int stay_time, in_addr_t ip, in_port_t port);