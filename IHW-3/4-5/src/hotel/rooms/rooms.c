#include "rooms.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
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
    for (unsigned int i = 0; i < 15; i++) { rooms.rooms[i].residents.people[0] = 0; rooms.rooms[i].residents.people[1] = 0; }
    for (unsigned int i = 15; i < 25; i++) { rooms.rooms[i].residents.person = 0; }
    rooms.ok = true;
    return rooms;
}
int close_rooms(struct Rooms* rooms)
{
    free(rooms->semaphore_name);
    free(rooms->memory_name);
    return close_semaphore(rooms->rooms_sync);
}
int destroy_rooms(struct Rooms* rooms)
{
    int err = 0;
    for (unsigned int i = 0; i < 15; i++)
    {
        if (rooms->rooms[i].residents.people[0])
        {
            if (kill(rooms->rooms[i].residents.people[0], SIGINT) == -1) err = errno;
        }
        if (rooms->rooms[i].residents.people[1])
        {
            if (kill(rooms->rooms[i].residents.people[1], SIGINT) == -1) err = errno;
        }
    }
    for (unsigned int i = 15; i < 25; i++)
    {
        if (rooms->rooms[i].residents.person)
        {
            if (kill(rooms->rooms[i].residents.person, SIGINT) == -1) err = errno;
        }
    }

    if (close_semaphore(rooms->rooms_sync) == -1) err = errno;
    if (delete_semaphore(rooms->semaphore_name) == -1) err = errno;
    if (delete_memory(rooms->memory_name) == -1) err = errno;
    free(rooms->memory_name);
    free(rooms->semaphore_name);

    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}



static int find_room(struct Rooms* rooms, enum Gender gender)
{
    for (unsigned int i = 0; i < 15; i++)
    {
        if (!rooms->rooms[i].residents.people[0] && !rooms->rooms[i].residents.people[1]) return i; // Empty room for two people
        if (rooms->rooms[i].residents.people[0] && !rooms->rooms[i].residents.people[1] && rooms->rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
        if (!rooms->rooms[i].residents.people[0] && rooms->rooms[i].residents.people[1] && rooms->rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
    }
    for (unsigned int i = 15; i < 25; i++)
    {
        if (!rooms->rooms[i].residents.person) return i; // Empty room for one person
    }
    return -1;
}
int take_room(struct Rooms* rooms, enum Gender gender)
{
    if (wait_semaphore(rooms->rooms_sync) == -1) { perror("Failed to wait on the semaphore"); return -1; }
    int room = find_room(rooms, gender); // Find a room to put the visitor to
    if (room >= 0 && room < 15)
    {
        rooms->rooms[room].gender = gender; // Set the gender of the room
        // Take one place
        if (!rooms->rooms[room].residents.people[0]) rooms->rooms[room].residents.people[0] = getpid();
        else rooms->rooms[room].residents.people[1] = getpid();
    }
    if (room >= 15 && room < 25)
    {
        rooms->rooms[room].gender = gender; // Set the gender of the room
        rooms->rooms[room].residents.person = getpid(); // Take the place
    }
    if (post_semaphore(rooms->rooms_sync) == -1) { perror("Failed to post the semaphore"); return -1; }
    return room;
}


int free_room(struct Rooms* rooms)
{
    // Find where the visitor has been living and remove the data about him
    for (unsigned int i = 0; i < 15; i++)
    {
        if (rooms->rooms[i].residents.people[0] == getpid()) { rooms->rooms[i].residents.people[0] = 0; return i; }
        if (rooms->rooms[i].residents.people[1] == getpid()) { rooms->rooms[i].residents.people[1] = 0; return i; }
    }
    for (unsigned int i = 15; i < 25; i++)
    {
        if (rooms->rooms[i].residents.person == getpid()) { rooms->rooms[i].residents.person = 0; return i; }
    }
    return -1;
}