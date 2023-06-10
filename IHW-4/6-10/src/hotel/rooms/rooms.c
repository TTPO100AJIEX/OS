#include "rooms.h"

#include <stddef.h>
#include <stdbool.h>

static const size_t ROOMS1 = 10;
static const size_t ROOMS2 = 15;
static struct Room rooms[ROOMS2 + ROOMS1];

void init_rooms()
{
    for (size_t i = 0; i < ROOMS2; i++)
    {
        rooms[i].gender = GENDER_NONE;
        rooms[i].residents.people[0] = (struct Resident){ .ip = 0, .port = 0 };
        rooms[i].residents.people[1] = (struct Resident){ .ip = 0, .port = 0 };
    }
    for (size_t i = ROOMS2; i < ROOMS1; i++)
    {
        rooms[i].gender = GENDER_NONE;
        rooms[i].residents.person = (struct Resident){ .ip = 0, .port = 0 };
    }
}
void destroy_rooms()
{
    // TODO: kick everytone out
}

static void print_id(unsigned int id)
{
    char offset = ' ';
    for (int denom = 10000; denom > 0; denom /= 10)
    {
        if (denom == 1 || id / denom != 0) offset = '0';
        printf("%c", offset + id / denom);
        id %= denom;
    }
}
void print_rooms()
{
    printf("-");
    for (size_t i = 0; i < ROOMS2 + ROOMS1; i++) for (size_t j = 0; j < 6; j++) printf("-");
    printf("\n");

    printf("|");
    for (size_t i = 0; i < ROOMS2; i++) { print_id(rooms[i].residents.people[0].id); printf("|"); }
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++) { print_id(rooms[i].residents.person.id); printf("|"); }
    printf("\n");

    printf("|");
    for (size_t i = 0; i < ROOMS2; i++) printf("  (%c)|", rooms[i].gender == GENDER_MALE ? 'm' : rooms[i].gender == GENDER_FEMALE ? 'f' : ' ');
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++) printf("-(%c)--", rooms[i].gender == GENDER_MALE ? 'm' : rooms[i].gender == GENDER_FEMALE ? 'f' : ' ');
    printf("\n");

    printf("|");
    for (size_t i = 0; i < ROOMS2; i++) { print_id(rooms[i].residents.people[1].id); printf("|"); }
    printf("\n");

    printf("-");
    for (size_t i = 0; i < ROOMS2; i++) for (size_t j = 0; j < 6; j++) printf("-");
    printf("\n");
}



static void free_rooms()
{
    // TODO: send leave_response to those whose time has passed
}
static int find_room(enum Gender gender)
{
    // Search rooms for two people
    for (size_t i = 0; i < ROOMS2; i++)
    {
        if (!rooms[i].residents.people[0].ip && !rooms[i].residents.people[1].ip) return i; // Empty room for two people
        if (rooms[i].residents.people[0] && !rooms[i].residents.people[1] && rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
        if (!rooms[i].residents.people[0] && rooms[i].residents.people[1] && rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
    }
    // Search rooms for one person
    for (int i = ROOMS2; i < ROOMS2 + ROOMS1; i++)
    {
        if (!rooms[i].residents.person) return i; // Empty room for one person
    }
    // Nothing has been found
    return -1;
}
int take_room(enum Gender gender, unsigned int stay_time, in_addr_t ip, in_port_t port)
{
    free_rooms();
    int room = find_room(this, gender); // Find a room to put the visitor to



    if (room != -1) this->storage[room].gender = gender; // Set the gender of the room
    if (room >= 0 && room < this->rooms2)
    {
        // Take one place
        if (!this->storage[room].residents.people[0]) this->storage[room].residents.people[0] = getpid();
        else this->storage[room].residents.people[1] = getpid();
    }
    if (room >= this->rooms2) this->storage[room].residents.person = getpid(); // Take the place
    return room;
}