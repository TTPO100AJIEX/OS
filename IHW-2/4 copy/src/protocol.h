#pragma once

#include <unistd.h>
#include <semaphore.h>

enum Gender { NONE, MALE, FEMALE };


struct ComeRequest
{
    enum Gender gender;
    pid_t id;
};
struct LeaveRequest
{
    pid_t id;
};

struct ComeResponse
{
    enum ComeStatus { COME_OK, COME_SORRY } status;
};
struct LeaveResponse
{
    enum LeaveStatus { LEAVE_OK } status;
};

enum MessageType { COME_REQUEST, LEAVE_REQUEST, COME_RESPONSE, LEAVE_RESPONSE };
struct Message
{
    enum MessageType type;
    union MessageData
    {
        struct ComeRequest come_request;
        struct LeaveRequest leave_request;

        struct ComeResponse come_response;
        struct LeaveResponse leave_response;
    } data;
};



#define door_in_semaphore "door_in_semaphore"
#define door_out_semaphore "door_out_semaphore"
#define reception_in_semaphore "reception_in_semaphore"
#define reception_out_semaphore "reception_out_semaphore"
#define memory "memory"
struct State
{
    sem_t* door_in;
    sem_t* door_out;
    sem_t* reception_in;
    sem_t* reception_out;
    struct Message* shared_memory;
};