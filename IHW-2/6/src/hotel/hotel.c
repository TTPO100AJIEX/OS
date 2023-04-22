#include "hotel.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include "../log/log.h"

// Room descriptor
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

static bool running = true;
static struct State state;
struct Room rooms[25];
static void stop()
{
    // If the signal has already been received, do nothing
    if (!running) return;
    running = false;
    
    // Kill all remaining visitors
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && rooms[i].residents.person)
        {
            if (kill(rooms[i].residents.person, SIGINT) == -1) printf("Hotel: failed to kill a visitor\n");
        }
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[0])
        {
            if (kill(rooms[i].residents.people[0], SIGINT) == -1) printf("Hotel: failed to kill a visitor\n");
        }
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[1])
        {
            if (kill(rooms[i].residents.people[1], SIGINT) == -1) printf("Hotel: failed to kill a visitor\n");
        }
    }

    // Close all the utilities and exit
    if (close_state(state) == -1)
    {
        printf("Visitor: failed to close the state\n");
        exit(1);
    }
    exit(0);
}



static int findRoomForGender(enum Gender gender)
{
    // Find a room to put the visitor to
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && !rooms[i].residents.person) return i; // If it is for one person and is empty
        if (!rooms[i].isForSinglePerson)
        {
            if (!rooms[i].residents.people[0] && !rooms[i].residents.people[1]) return i; // If it is for two people and is empty
            if (rooms[i].residents.people[0] && !rooms[i].residents.people[1] && rooms[i].gender == gender) return i; // If it is for two people, there is an empty place and the gender matches
            if (!rooms[i].residents.people[0] && rooms[i].residents.people[1] && rooms[i].gender == gender) return i; // If it is for two people, there is an empty place and the gender matches
        }
    }
    return -1; // Nothing has been found
}
static void registerRoom(enum Gender gender, pid_t id, int room)
{
    rooms[room].gender = gender; // Set the gender of the room
    if (rooms[room].isForSinglePerson) { rooms[room].residents.person = id; return; } // Fill the only place in the room
    // Fill one of the places in the room for two
    if (!rooms[room].residents.people[0]) { rooms[room].residents.people[0] = id; return; }
    rooms[room].residents.people[1] = id;
}
static int handleCome(struct ComeRequest request)
{
    log("Hotel: %s visitor %d wants to enter\n", request.gender == MALE ? "male" : "female", request.id);
    int room = findRoomForGender(request.gender); // Find the room for this person
    if (room == -1)
    {
        // If nothing has been found
        log("Hotel: failed to register the visitor %d\n", request.id);
        // Construct the response message
        struct Message message = {
            .type = COME_RESPONSE,
            .data = { .come_response = { COME_SORRY } }
        };
        return response(state, message); // Send the response
    }
    else
    {
        // A room has been found
        registerRoom(request.gender, request.id, room); // Put the visitor to the room
        log("Hotel: registered the visitor %d into the room %d\n", request.id, room);
        // Construct the response message
        struct Message message = {
            .type = COME_RESPONSE,
            .data = { .come_response = { COME_OK } }
        };
        return response(state, message); // Send the response
    }
}

static int handleLeave(struct LeaveRequest request)
{
    log("Hotel: visitor with id %d wants to leave\n", request.id);

    // Find where the visitor has been living and remove the data about him
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && rooms[i].residents.person == request.id) rooms[i].residents.person = 0;
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[0] == request.id) rooms[i].residents.people[0] = 0;
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[1] == request.id) rooms[i].residents.people[1] = 0;
    }

    log("Hotel: removed visitor %d\n", request.id);
    // Construct the response message
    struct Message message = {
        .type = LEAVE_RESPONSE,
        .data = { .leave_response = { LEAVE_OK } }
    };
    return response(state, message); // Send the response
}

int hotel(struct State l_state)
{
    state = l_state;
    signal(SIGINT, stop); // Register the signal handler
    log("Hotel: started with PID %d\n", getpid());
    // Setup the rooms
    for (unsigned int i = 0; i < 15; i++) { rooms[i].isForSinglePerson = false; rooms[i].residents.people[0] = 0; rooms[i].residents.people[1] = 0; }
    for (unsigned int i = 15; i < 25; i++) { rooms[i].isForSinglePerson = true; rooms[i].gender = NONE; rooms[i].residents.person = 0; }
    log("Hotel: initialized %d rooms\n", 25);

    while (running)
    {
        // Wait for the next request and get it
        struct Message message;
        if (get_request(state, &message) == -1) printf("Hotel: failed to get the request\n");
        // Check the type of the request and do the required handling
        switch (message.type)
        {
            case COME_REQUEST:
            {
                if (handleCome(message.data.come_request) == -1) printf("Hotel: failed to process a come request\n");
                break;
            }
            case LEAVE_REQUEST:
            {
                if (handleLeave(message.data.leave_request) == -1) printf("Hotel: failed to process a leave request\n");
                break;
            }
            default: { }
        }
    }

    return 0;
}