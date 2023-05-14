#pragma once

#include <stdbool.h>
#include <sys/types.h>

enum Gender { NONE, MALE, FEMALE };

struct Room
{
    bool isForSinglePerson;
    enum Gender gender;
    union RoomResidents
    {
        pid_t person;
        pid_t people[2];
    } residents;
};