#include "rooms.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>

static const size_t ROOMS1 = 10;
static const size_t ROOMS2 = 15;
static struct Room rooms[ROOMS2 + ROOMS1];

extern int server;
static void force_leave(struct Resident resident)
{
    // Send the message to the visitor
    struct Response res = { .type = LEAVE_RESPONSE, .data = { .leave = { } } };
    if (sendto(server, &res, sizeof(res), 0, (struct sockaddr *)(&resident.client), sizeof(resident.client)) != sizeof(res)) perror("Failed to send a response");
}

void init_rooms()
{
    for (size_t i = 0; i < ROOMS2; i++)
    {
        rooms[i].id = i + 1;
        rooms[i].gender = GENDER_NONE;
        rooms[i].residents.people[0].id = rooms[i].residents.people[1].id = 0;
    }
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++)
    {
        rooms[i].id = i + 1;
        rooms[i].gender = GENDER_NONE;
        rooms[i].residents.person.id = 0;
    }
}
void destroy_rooms()
{
    for (size_t i = 0; i < ROOMS2; i++)
    {
        for (size_t j = 0; j < 2; j++)
        {
            if (rooms[i].residents.people[j].id)
            {
                rooms[i].residents.people[j].id = 0;
                force_leave(rooms[i].residents.people[j]);
            }
        }
    }
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++)
    {
        if (rooms[i].residents.person.id)
        {
            rooms[i].residents.person.id = 0;
            force_leave(rooms[i].residents.person);
        }
    }
}

static void print_id(size_t id)
{
    if (id < 10) printf(" %zu ", id);
    if (id >= 10 && id < 100) printf(" %zu", id);
    if (id >= 100) printf("%zu", id % 1000);
}
void print_rooms()
{
    printf("-");
    for (size_t i = 0; i < ROOMS2 + ROOMS1; i++) for (size_t j = 0; j < 4; j++) printf("-");
    printf("\n");

    printf("|");
    for (size_t i = 0; i < ROOMS2; i++) { print_id(rooms[i].residents.people[0].id); printf("|"); }
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++) { print_id(rooms[i].residents.person.id); printf("|"); }
    printf("\n");

    printf("|");
    for (size_t i = 0; i < ROOMS2; i++) printf("(%c)|", rooms[i].gender == GENDER_MALE ? 'm' : rooms[i].gender == GENDER_FEMALE ? 'f' : ' ');
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++) printf("(%c)|", rooms[i].gender == GENDER_MALE ? 'm' : rooms[i].gender == GENDER_FEMALE ? 'f' : ' ');
    printf("\n");

    printf("|");
    for (size_t i = 0; i < ROOMS2; i++) { print_id(rooms[i].residents.people[1].id); printf("|"); }
    printf("\n");

    printf("-");
    for (size_t i = 0; i < ROOMS2; i++) for (size_t j = 0; j < 4; j++) printf("-");
    printf("\n");
}



static long int compare_timstamps(struct timeval first, struct timeval second)
{
    if (first.tv_sec == second.tv_sec) return first.tv_usec - second.tv_usec;
    return first.tv_sec - second.tv_sec;
}
static void free_rooms()
{
    // Get current time
    struct timeval cur_time;
    if (gettimeofday(&cur_time, NULL) == -1) return;

    for (size_t i = 0; i < ROOMS2; i++)
    {
        for (size_t j = 0; j < 2; j++)
        {
            if (rooms[i].residents.people[j].id && compare_timstamps(cur_time, rooms[i].residents.people[j].leave_time) > 0)
            {
                rooms[i].residents.people[j].id = 0;
                force_leave(rooms[i].residents.people[j]);
            }
        }
    }
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++)
    {
        if (rooms[i].residents.person.id && compare_timstamps(cur_time, rooms[i].residents.person.leave_time) > 0)
        {
            rooms[i].residents.person.id = 0;
            force_leave(rooms[i].residents.person);
        }
    }
}
static struct Room* find_room(enum Gender gender)
{
    // Search rooms for two people
    for (size_t i = 0; i < ROOMS2; i++)
    {
        if (!rooms[i].residents.people[0].id && !rooms[i].residents.people[1].id) return &rooms[i]; // Empty room for two people
        if (rooms[i].residents.people[0].id && !rooms[i].residents.people[1].id && rooms[i].gender == gender) return &rooms[i]; // Empty place in a room for two people and the gender matches
        if (!rooms[i].residents.people[0].id && rooms[i].residents.people[1].id && rooms[i].gender == gender) return &rooms[i]; // Empty place in a room for two people and the gender matches
    }
    // Search rooms for one person
    for (size_t i = ROOMS2; i < ROOMS2 + ROOMS1; i++)
    {
        if (!rooms[i].residents.person.id) return &rooms[i]; // Empty room for one person
    }
    // Nothing has been found
    return NULL;
}
const struct Room* take_room(size_t id, enum Gender gender, unsigned int stay_time, struct sockaddr_in client)
{
    free_rooms(); // Kick everyone whose time has passed
    struct Room* room = find_room(gender); // Find a room to put the visitor into
    if (!room) return NULL; // Nothing has been found

    // Calculate the leave timestamp based on current timestamp
    struct timeval leave_time;
    if (gettimeofday(&leave_time, NULL) == -1) return NULL;
    leave_time.tv_sec += stay_time + 1; // One extra second for better timing

    room->gender = gender;
    struct Resident resident = { .id = id, .leave_time = leave_time, .client = client };
    if (room->id <= ROOMS2)
    {
        if (!room->residents.people[0].id) room->residents.people[0] = resident;
        else room->residents.people[1] = resident;
    }
    else
    {
        room->residents.person = resident;
    }
    return room;
}