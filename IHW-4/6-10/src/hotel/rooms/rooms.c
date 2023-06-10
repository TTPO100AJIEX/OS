#include "rooms.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static long int compare_timstamps(struct timeval first, struct timeval second)
{
    if (first.tv_sec == second.tv_sec) return first.tv_usec - second.tv_usec;
    return first.tv_sec - second.tv_sec;
}


extern int server;
static struct Room rooms[25];
void init_rooms()
{
    for (size_t i = 0; i < 15; i++) rooms[i] = (struct Room){ .id = i + 1, .gender = GENDER_NONE, .max_residents = 2, .residents = calloc(2, sizeof(struct Resident)) };
    for (size_t i = 15; i < 25; i++) rooms[i] = (struct Room){ .id = i + 1, .gender = GENDER_NONE, .max_residents = 1, .residents = calloc(1, sizeof(struct Resident)) };
}
static void force_leave(size_t room_index, size_t resident_index)
{
    send_response(server, (struct Response){ .type = LEAVE_RESPONSE, .data = { .leave = { } } }, rooms[room_index].residents[resident_index].client);
    printf("Visitor %zu forced to leave the room %zu\n", rooms[room_index].residents[resident_index].id, rooms[room_index].id);
    rooms[room_index].residents[resident_index].id = 0;
}
static void free_rooms()
{
    // Get current time
    struct timeval cur_time;
    if (gettimeofday(&cur_time, NULL) == -1) return;
    
    for (size_t i = 0; i < 25; i++)
    {
        for (size_t j = 0; j < rooms[i].max_residents; j++)
        {
            if (!rooms[i].residents[j].id || compare_timstamps(cur_time, rooms[i].residents[j].leave_time) < 0) continue;
            force_leave(i, j); // Force the visitor to leave
        }
    }
}
void destroy_rooms()
{
    for (size_t i = 0; i < 25; i++)
    {
        for (size_t j = 0; j < rooms[i].max_residents; j++)
        {
            if (!rooms[i].residents[j].id) continue;
            force_leave(i, j); // Force the visitor to leave
        }
        free(rooms[i].residents); // Free the dynamic memory
    }
}


static struct Room* find_room(enum Gender gender)
{
    for (size_t i = 0; i < 25; i++)
    {
        if (rooms[i].gender != gender && rooms[i].gender != GENDER_NONE) continue;
        for (size_t j = 0; j < rooms[i].max_residents; j++)
        {
            if (!rooms[i].residents[j].id) return &rooms[i];
        }
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

    // Save the information
    room->gender = gender;
    for (size_t i = 0; i < room->max_residents; i++)
    {
        if (room->residents[i].id) continue;
        room->residents[i] = (struct Resident){ .id = id, .leave_time = leave_time, .client = client };
        return room;
    }
    return NULL;
}

const struct Room* free_room(size_t room_id, size_t visitor_id)
{
    if (room_id > 25) return NULL; // Invalid input
    struct Room* room = &rooms[room_id - 1];
    bool has_residents = false, found_visitor = false;
    for (size_t i = 0; i < room->max_residents; i++)
    {
        if (room->residents[i].id == visitor_id)
        {
            // Visitor found
            found_visitor = true;
            room->residents[i].id = 0;
        }
        if (!has_residents) has_residents = room->residents[i].id; // Update the flag
    }
    if (!has_residents) room->gender = GENDER_NONE; // Reset the gender if noone leaves in the room
    return found_visitor ? room : NULL;
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
    for (size_t i = 0; i < 25; i++) for (size_t j = 0; j < 4; j++) printf("-");
    printf("\n");

    printf("|");
    for (size_t i = 0; i < 25; i++) { print_id(rooms[i].residents[0].id); printf("|"); }
    printf("\n");

    printf("|");
    for (size_t i = 0; i < 25; i++) printf("(%c)|", rooms[i].gender == GENDER_MALE ? 'm' : rooms[i].gender == GENDER_FEMALE ? 'f' : ' ');
    printf("\n");

    printf("|");
    for (size_t i = 0; i < 15; i++) { print_id(rooms[i].residents[1].id); printf("|"); }
    printf("\n");

    printf("-");
    for (size_t i = 0; i < 15; i++) for (size_t j = 0; j < 4; j++) printf("-");
    printf("\n");
}