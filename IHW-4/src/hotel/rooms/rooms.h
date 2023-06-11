#pragma once

#include <sys/time.h>
#include "../../protocol.h"

struct Resident
{
    size_t id;
    struct timeval leave_time;
    struct sockaddr_in client;
};
struct Room
{
    size_t id;
    enum Gender gender;
    size_t max_residents;
    struct Resident* residents;
};

void init_rooms();
void destroy_rooms();

const struct Room* take_room(size_t id, enum Gender gender, unsigned int stay_time, struct sockaddr_in client);
const struct Room* free_room(size_t room_id, size_t visitor_id);

char* get_rooms_layout();