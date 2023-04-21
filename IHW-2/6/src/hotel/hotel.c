#include "hotel.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include "../log/log.h"

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
    if (!running) return;
    running = false;
    
    // Kill all remaining visitors
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && rooms[i].residents.person)
        {
            if (kill(rooms[i].residents.person, SIGINT) == -1) perror("Hotel: failed to kill a visitor");
        }
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[0])
        {
            if (kill(rooms[i].residents.people[0], SIGINT) == -1) perror("Hotel: failed to kill a visitor");
        }
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[1])
        {
            if (kill(rooms[i].residents.people[1], SIGINT) == -1) perror("Hotel: failed to kill a visitor");
        }
    }

    if (close_state(state) == -1)
    {
        perror("Visitor: failed to close the state");
        exit(1);
    }
    exit(0);
}



static int findRoomForGender(enum Gender gender)
{
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && !rooms[i].residents.person) return i;
        if (!rooms[i].isForSinglePerson)
        {
            if (!rooms[i].residents.people[0] && !rooms[i].residents.people[1]) return i;
            if (rooms[i].residents.people[0] && !rooms[i].residents.people[1] && rooms[i].gender == gender) return i;
            if (!rooms[i].residents.people[0] && rooms[i].residents.people[1] && rooms[i].gender == gender) return i;
        }
    }
    return -1;
}
static void registerRoom(enum Gender gender, pid_t id, int room)
{
    rooms[room].gender = gender;
    if (rooms[room].isForSinglePerson) { rooms[room].residents.person = id; return; }
    if (!rooms[room].residents.people[0]) { rooms[room].residents.people[0] = id; return; }
    rooms[room].residents.people[1] = id;
}
static int handleCome(struct ComeRequest request)
{
    log("Hotel: %s visitor with id %d came\n", request.gender == MALE ? "male" : "female", request.id);
    int room = findRoomForGender(request.gender);
    if (room == -1)
    {
        struct Message message = {
            .type = COME_RESPONSE,
            .data = { .come_response = { COME_SORRY } }
        };
        log("Hotel: failed to register visitor %d\n", request.id);
        return response(state, message);
    }
    else
    {
        registerRoom(request.gender, request.id, room);
        struct Message message = {
            .type = COME_RESPONSE,
            .data = { .come_response = { COME_OK } }
        };
        log("Hotel: registered visitor %d\n", request.id);
        return response(state, message);
    }
}

static int handleLeave(struct LeaveRequest request)
{
    log("Hotel: visitor with id %d wants to leave\n", request.id);

    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && rooms[i].residents.person == request.id) rooms[i].residents.person = 0;
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[0] == request.id) rooms[i].residents.people[0] = 0;
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[1] == request.id) rooms[i].residents.people[1] = 0;
    }

    struct Message message = {
        .type = LEAVE_RESPONSE,
        .data = { .leave_response = { LEAVE_OK } }
    };
    log("Hotel: removed visitor %d\n", request.id);
    return response(state, message);
}

int hotel(struct State l_state)
{
    state = l_state;
    signal(SIGINT, stop);
    log("Hotel: started with PID %d\n", getpid());

    for (unsigned int i = 0; i < 15; i++) { rooms[i].isForSinglePerson = false; rooms[i].residents.people[0] = 0; rooms[i].residents.people[1] = 0; }
    for (unsigned int i = 15; i < 25; i++) { rooms[i].isForSinglePerson = true; rooms[i].gender = NONE; rooms[i].residents.person = 0; }
    log("Hotel: initialized %d rooms\n", 25);

    while (running)
    {
        if (post_semaphore(state.door_in) == -1) perror("Hotel: failed to open the door_in semaphore");
        if (wait_semaphore(state.reception_in) == -1) perror("Hotel: failed to close the reception_in semaphore");

        switch (state.shared_memory->type)
        {
            case COME_REQUEST:
            {
                if (handleCome(state.shared_memory->data.come_request) == -1) perror("Hotel: failed to process a come request");
                break;
            }
            case LEAVE_REQUEST:
            {
                if (handleLeave(state.shared_memory->data.leave_request) == -1) perror("Hotel: failed to process a leave request");
                break;
            }
            default: { }
        }
    }

    return 0;
}