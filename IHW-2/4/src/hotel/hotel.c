#include "hotel.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include "../log/log.h"
#include "../utils/utils.h"

static bool running = true;
static sem_t* to_unlock;
static void stop()
{
    running = false;
    if (sem_post(to_unlock) == -1) perror("Hotel: failed to open the to_unlock semaphore");
}

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

static void handleCome(struct ComeRequest request, struct State state, struct Room rooms[25])
{
    log("Hotel: %s visitor with id %d came\n", request.gender == MALE ? "male" : "female", request.id);

    struct Response response = { COME_RESPONSE, .data = { .come_response = { COME_SORRY } } };
    int room = -1;
    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && !rooms[i].residents.person)
        {
            room = i;
            rooms[i].gender = request.gender;
            rooms[i].residents.person = request.id;
            break;
        }
        if (!rooms[i].isForSinglePerson)
        {
            if (!rooms[i].residents.people[0] && !rooms[i].residents.people[1])
            {
                room = i;
                rooms[i].gender = request.gender;
                rooms[i].residents.people[0] = request.id;
                break;
            }
            if (rooms[i].residents.people[0] && !rooms[i].residents.people[1] && rooms[i].gender == request.gender)
            {
                room = i;
                rooms[i].residents.people[1] = request.id;
                break;
            }
            if (!rooms[i].residents.people[0] && rooms[i].residents.people[1] && rooms[i].gender == request.gender)
            {
                room = i;
                rooms[i].residents.people[0] = request.id;
                break;
            }
        }
    }

    sleep_milliseconds(250);
    if (room == -1)
    {
        log("Hotel: failed to register visitor %d\n", request.id);
    }
    else
    {
        response.data.come_response.status = COME_OK;
        log("Hotel: registered visitor %d\n", request.id);
    }
    state.shared_memory->type = RESPONSE;
    state.shared_memory->data.response = response;
    if (sem_post(state.reception_out) == -1) perror("Hotel: failed to open the reception_out semaphore");
}
static void handleLeave(struct LeaveRequest request, struct State state, struct Room rooms[25])
{
    log("Hotel: visitor with id %d wants to leave\n", request.id);

    for (unsigned int i = 0; i < 25; i++)
    {
        if (rooms[i].isForSinglePerson && rooms[i].residents.person == request.id) rooms[i].residents.person = 0;
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[0] == request.id) rooms[i].residents.people[0] = 0;
        if (!rooms[i].isForSinglePerson && rooms[i].residents.people[1] == request.id) rooms[i].residents.people[1] = 0;
    }

    struct Response response = { LEAVE_RESPONSE, .data = { .leave_response = { LEAVE_OK } } };
    state.shared_memory->type = RESPONSE;
    state.shared_memory->data.response = response;

    log("Hotel: removed visitor %d\n", request.id);
    if (sem_post(state.reception_out) == -1) perror("Hotel: failed to open the reception_out semaphore");
}

int hotel(struct State state)
{
    to_unlock = state.reception_in;
    signal(SIGINT, stop);
    log("Hotel: started with PID %d\n", getpid());

    struct Room hotel[25];
    for (unsigned int i = 0; i < 15; i++)
    {
        hotel[i].isForSinglePerson = false;
        hotel[i].gender = NONE;
        hotel[i].residents.people[0] = 0;
        hotel[i].residents.people[1] = 0;
    }
    for (unsigned int i = 15; i < 25; i++)
    {
        hotel[i].isForSinglePerson = true;
        hotel[i].gender = NONE;
        hotel[i].residents.person = 0;
    }
    log("Hotel: initialized %d rooms\n", 25);

    while (running)
    {
        if (sem_post(state.door) == -1) perror("Hotel: failed to open the door semaphore");
        if (sem_wait(state.reception_in) == -1) perror("Hotel: failed to close the reception_in semaphore");
        if (state.shared_memory->type == RESPONSE) continue;

        struct Request request = state.shared_memory->data.request;
        switch (request.type)
        {
            case COME_REQUEST: { handleCome(request.data.come_request, state, hotel); break; }
            case LEAVE_REQUEST: { handleLeave(request.data.leave_request, state, hotel); break; }
        }
    }
    
    for (unsigned int i = 0; i < 25; i++)
    {
        if (hotel[i].isForSinglePerson)
        {
            if (hotel[i].residents.person)
            {
                if (kill(hotel[i].residents.person, SIGINT) == -1) perror("Hotel: failed to kill a visitor");
            }
        }
        else
        {
            if (hotel[i].residents.people[0])
            {
                if (kill(hotel[i].residents.people[0], SIGINT) == -1) perror("Hotel: failed to kill a visitor");
            }
            if (hotel[i].residents.people[1])
            {
                if (kill(hotel[i].residents.people[1], SIGINT) == -1) perror("Hotel: failed to kill a visitor");
            }
        }
    }

    if (sem_close(state.door) == -1) perror("Hotel: failed to close the door semaphore");
    if (sem_close(state.reception_in) == -1) perror("Hotel: failed to close the reception_in semaphore");
    if (sem_close(state.reception_out) == -1) perror("Hotel: failed to close the reception_out semaphore");
    if (stop_logging() == -1) perror("Hotel: failed to stop the logging");
    return 0;
}