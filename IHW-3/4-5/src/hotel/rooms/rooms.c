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
    // Initialize all fields and return { ok = false } if something failed
    struct Rooms rooms = { .ok = false };
    if (!(rooms.memory_name = malloc(strlen(memory_name) + 1))) return rooms;
    if (!(rooms.rooms = create_memory(memory_name, 25 * sizeof(struct Room)))) return rooms;
    if (!(rooms.semaphore_name = malloc(strlen(semaphore_name) + 1))) return rooms;
    if (!(rooms.rooms_sync = create_semaphore(semaphore_name, 1))) return rooms;
    // Save memory and semaphore names
    strcpy(rooms.memory_name, memory_name);
    strcpy(rooms.semaphore_name, semaphore_name);
    // Initialize the rooms
    for (unsigned int i = 0; i < 15; i++) { rooms.rooms[i].residents.people[0] = 0; rooms.rooms[i].residents.people[1] = 0; }
    for (unsigned int i = 15; i < 25; i++) { rooms.rooms[i].residents.person = 0; }
    // Initialization completed successfully
    rooms.ok = true;
    return rooms;
}
int close_rooms(struct Rooms* rooms)
{
    // Free the memory
    free(rooms->semaphore_name);
    free(rooms->memory_name);
    // Close the semaphore
    return close_semaphore(rooms->rooms_sync);
}
int destroy_rooms(struct Rooms* rooms)
{
    int err = 0; // Save the error for perror to work correctly with this function
    // Kill all remaining visitors
    for (unsigned int i = 0; i < 15; i++)
    {
        if (rooms->rooms[i].residents.people[0] && kill(rooms->rooms[i].residents.people[0], SIGINT) == -1) err = errno;
        if (rooms->rooms[i].residents.people[1] && kill(rooms->rooms[i].residents.people[1], SIGINT) == -1) err = errno;
    }
    for (unsigned int i = 15; i < 25; i++)
    {
        if (rooms->rooms[i].residents.person && kill(rooms->rooms[i].residents.person, SIGINT) == -1) err = errno;
    }

    // Close and delete the semaphore
    if (close_semaphore(rooms->rooms_sync) == -1) err = errno;
    if (delete_semaphore(rooms->semaphore_name) == -1) err = errno;
    // Delete the memory
    if (delete_memory(rooms->memory_name) == -1) err = errno;
    // Free the memory occupied by memory and semaphore names
    free(rooms->memory_name);
    free(rooms->semaphore_name);

    // Restore the error if needed and return
    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}



static int find_room(struct Rooms* rooms, enum Gender gender)
{
    // Search rooms for two people
    for (unsigned int i = 0; i < 15; i++)
    {
        if (!rooms->rooms[i].residents.people[0] && !rooms->rooms[i].residents.people[1]) return i; // Empty room for two people
        if (rooms->rooms[i].residents.people[0] && !rooms->rooms[i].residents.people[1] && rooms->rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
        if (!rooms->rooms[i].residents.people[0] && rooms->rooms[i].residents.people[1] && rooms->rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
    }
    // Search rooms for one person
    for (unsigned int i = 15; i < 25; i++)
    {
        if (!rooms->rooms[i].residents.person) return i; // Empty room for one person
    }
    // Nothing has been found
    return -1;
}
int take_room(struct Rooms* rooms, enum Gender gender)
{
    if (wait_semaphore(rooms->rooms_sync) == -1) { perror("take_room: failed to wait on the semaphore"); return -1; } // Lock access to the rooms
    int room = find_room(rooms, gender); // Find a room to put the visitor to
    if (room != -1) rooms->rooms[room].gender = gender; // Set the gender of the room
    if (room >= 0 && room < 15)
    {
        // Take one place
        if (!rooms->rooms[room].residents.people[0]) rooms->rooms[room].residents.people[0] = getpid();
        else rooms->rooms[room].residents.people[1] = getpid();
    }
    if (room >= 15 && room < 25) rooms->rooms[room].residents.person = getpid(); // Take the place
    if (post_semaphore(rooms->rooms_sync) == -1) { perror("take_room: failed to post the semaphore"); return -1; } // Unlock access to the rooms
    return room;
}


int free_room(struct Rooms* rooms)
{
    if (wait_semaphore(rooms->rooms_sync) == -1) { perror("free_room: failed to wait on the semaphore"); return -1; } // Lock access to the rooms
    // Find where the visitor has been living and remove the data about him
    int answer = -1;
    // Check rooms for two people
    for (unsigned int i = 0; i < 15; i++)
    {
        if (rooms->rooms[i].residents.people[0] == getpid())
        {
            rooms->rooms[i].residents.people[0] = 0;
            if (!rooms->rooms[i].residents.people[1]) rooms->rooms[i].gender = NONE;
            answer = i;
        }
        if (rooms->rooms[i].residents.people[1] == getpid())
        {
            rooms->rooms[i].residents.people[1] = 0;
            if (!rooms->rooms[i].residents.people[0]) rooms->rooms[i].gender = NONE;
            answer = i;
        }
    }
    // Check rooms for one person
    for (unsigned int i = 15; i < 25; i++)
    {
        if (rooms->rooms[i].residents.person == getpid()) { rooms->rooms[i].residents.person = 0; rooms->rooms[i].gender = NONE; answer = i; }
    }
    if (post_semaphore(rooms->rooms_sync) == -1) { perror("free_room: failed to post the semaphore"); return -1; } // Unlock access to the rooms
    return answer;
}



// Just some drawing
static char* draw_id(char* write_iter, pid_t id)
{
    char offset = ' ';
    for (unsigned int denom = 10000; denom > 0; denom /= 10)
    {
        if (denom == 1 || id / denom != 0) offset = '0';
        *(write_iter++) = offset + id / denom;
        id %= denom;
    }
    return write_iter;
}
static char* draw_gender(char* write_iter, enum Gender gender)
{
    *(write_iter++) = '(';
    switch (gender)
    {
        case MALE: { *(write_iter++) = 'm'; break; }
        case FEMALE: { *(write_iter++) = 'f'; break; }
        default: { *(write_iter++) = ' '; }
    }
    *(write_iter++) = ')';
    return write_iter;
}
char* get_rooms_layout(__attribute__ ((unused)) struct Rooms* rooms)
{
    char* result = malloc((25 * 6 + 2) * 3 + (15 * 6 + 2) * 2);
    char* write_iter = result;

    *(write_iter++) = '-';
    for (unsigned int i = 0; i < 25; i++) for (unsigned int j = 0; j < 6; j++) *(write_iter++) = '-';
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (unsigned int i = 0; i < 15; i++) { write_iter = draw_id(write_iter, rooms->rooms[i].residents.people[0]); *(write_iter++) = '|'; }
    for (unsigned int i = 15; i < 25; i++) { write_iter = draw_id(write_iter, rooms->rooms[i].residents.person); *(write_iter++) = '|'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (unsigned int i = 0; i < 15; i++) { *(write_iter++) = ' '; *(write_iter++) = ' '; write_iter = draw_gender(write_iter, rooms->rooms[i].gender); *(write_iter++) = '|'; }
    for (unsigned int i = 15; i < 25; i++) { *(write_iter++) = '-'; write_iter = draw_gender(write_iter, rooms->rooms[i].gender); *(write_iter++) = '-'; *(write_iter++) = '-'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (unsigned int i = 0; i < 15; i++) { write_iter = draw_id(write_iter, rooms->rooms[i].residents.people[1]); *(write_iter++) = '|'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '-';
    for (unsigned int i = 0; i < 15; i++) for (unsigned int j = 0; j < 6; j++) *(write_iter++) = '-';

    *(write_iter++) = '\0';
    return result;
}