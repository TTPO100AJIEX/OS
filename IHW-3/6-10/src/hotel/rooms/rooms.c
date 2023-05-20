#include "rooms.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

struct Rooms initialize_rooms(struct Room* rooms_storage, int rooms1, int rooms2)
{
    // Initialize the object
    struct Rooms rooms = { .owner = getpid(), .rooms = rooms_storage, .rooms1 = rooms1, .rooms2 = rooms2 };
    // Fill the memory
    for (int i = 0; i < rooms2; i++) { rooms.rooms[i].residents.people[0] = 0; rooms.rooms[i].residents.people[1] = 0; }
    for (int i = rooms2; i < rooms1 + rooms2; i++) { rooms.rooms[i].residents.person = 0; }
    return rooms;
}
int delete_rooms(struct Rooms* rooms)
{
    if (rooms->owner != getpid()) return 0; // Child processes do not need to do anything

    int err = 0; // Save the error for perror to work correctly with this function
    // Kill all remaining visitors
    for (int i = 0; i < rooms->rooms2; i++)
    {
        if (rooms->rooms[i].residents.people[0] && kill(rooms->rooms[i].residents.people[0], SIGINT) == -1) err = errno;
        if (rooms->rooms[i].residents.people[1] && kill(rooms->rooms[i].residents.people[1], SIGINT) == -1) err = errno;
    }
    for (int i = rooms->rooms2; i < rooms->rooms1 + rooms->rooms2; i++)
    {
        if (rooms->rooms[i].residents.person && kill(rooms->rooms[i].residents.person, SIGINT) == -1) err = errno;
    }
    // Restore the error if needed and return
    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}



// Just some nice drawing
static char* draw_id(char* write_iter, pid_t id)
{
    char offset = ' ';
    for (int denom = 10000; denom > 0; denom /= 10)
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
char* get_rooms_layout(struct Rooms* rooms)
{
    char* result = malloc(((rooms->rooms1 + rooms->rooms2) * 6 + 2) * 3 + (rooms->rooms2 * 6 + 2) * 2 + 1);
    char* write_iter = result;

    *(write_iter++) = '-';
    for (int i = 0; i < rooms->rooms1 + rooms->rooms2; i++) for (int j = 0; j < 6; j++) *(write_iter++) = '-';
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (int i = 0; i < rooms->rooms2; i++) { write_iter = draw_id(write_iter, rooms->rooms[i].residents.people[0]); *(write_iter++) = '|'; }
    for (int i = rooms->rooms2; i < rooms->rooms1 + rooms->rooms2; i++) { write_iter = draw_id(write_iter, rooms->rooms[i].residents.person); *(write_iter++) = '|'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (int i = 0; i < rooms->rooms2; i++) { *(write_iter++) = ' '; *(write_iter++) = ' '; write_iter = draw_gender(write_iter, rooms->rooms[i].gender); *(write_iter++) = '|'; }
    for (int i = rooms->rooms2; i < rooms->rooms1 + rooms->rooms2; i++) { *(write_iter++) = '-'; write_iter = draw_gender(write_iter, rooms->rooms[i].gender); *(write_iter++) = '-'; *(write_iter++) = '-'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '|';
    for (int i = 0; i < rooms->rooms2; i++) { write_iter = draw_id(write_iter, rooms->rooms[i].residents.people[1]); *(write_iter++) = '|'; }
    *(write_iter++) = '\n';

    *(write_iter++) = '-';
    for (int i = 0; i < rooms->rooms2; i++) for (int j = 0; j < 6; j++) *(write_iter++) = '-';

    *(write_iter++) = '\n'; *(write_iter++) = '\0';
    return result;
}



static int find_room(struct Rooms* rooms, enum Gender gender)
{
    // Search rooms for two people
    for (int i = 0; i < rooms->rooms2; i++)
    {
        if (!rooms->rooms[i].residents.people[0] && !rooms->rooms[i].residents.people[1]) return i; // Empty room for two people
        if (rooms->rooms[i].residents.people[0] && !rooms->rooms[i].residents.people[1] && rooms->rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
        if (!rooms->rooms[i].residents.people[0] && rooms->rooms[i].residents.people[1] && rooms->rooms[i].gender == gender) return i; // Empty place in a room for two people and the gender matches
    }
    // Search rooms for one person
    for (int i = rooms->rooms2; i < rooms->rooms1 + rooms->rooms2; i++)
    {
        if (!rooms->rooms[i].residents.person) return i; // Empty room for one person
    }
    // Nothing has been found
    return -1;
}
int take_room(struct Rooms* rooms, enum Gender gender)
{
    int room = find_room(rooms, gender); // Find a room to put the visitor to
    if (room != -1) rooms->rooms[room].gender = gender; // Set the gender of the room
    if (room >= 0 && room < rooms->rooms2)
    {
        // Take one place
        if (!rooms->rooms[room].residents.people[0]) rooms->rooms[room].residents.people[0] = getpid();
        else rooms->rooms[room].residents.people[1] = getpid();
    }
    if (room >= rooms->rooms2) rooms->rooms[room].residents.person = getpid(); // Take the place
    return room;
}


int free_room(struct Rooms* rooms)
{
    // Find where the visitor has been living and remove the data about him
    int answer = -1;
    // Check rooms for two people
    for (int i = 0; i < rooms->rooms2; i++)
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
    for (int i = rooms->rooms2; i < rooms->rooms1 + rooms->rooms2; i++)
    {
        if (rooms->rooms[i].residents.person == getpid()) { rooms->rooms[i].residents.person = 0; rooms->rooms[i].gender = NONE; answer = i; }
    }
    return answer;
}