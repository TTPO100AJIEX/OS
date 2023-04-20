#pragma once

#include <unistd.h>
#include <semaphore.h>

enum Gender { NONE, MALE, FEMALE };


enum RequestType { COME_REQUEST, LEAVE_REQUEST };
struct ComeRequest
{
    enum Gender gender;
    pid_t id;
};
struct LeaveRequest
{
    pid_t id;
};
struct Request
{
    enum RequestType type;
    union RequestData
    {
        struct ComeRequest come_request;
        struct LeaveRequest leave_request;
    } data;
};


enum ResponseType { COME_RESPONSE, LEAVE_RESPONSE };
struct ComeResponse
{
    enum ComeStatus { COME_OK, COME_SORRY } status;
};
struct LeaveResponse
{
    enum LeaveStatus { LEAVE_OK } status;
};
struct Response
{
    enum ResponseType type;
    union ResponseData
    {
        struct ComeResponse come_response;
        struct LeaveResponse leave_response;
    } data;
};


enum MessageType { REQUEST, RESPONSE };
struct Message
{
    enum MessageType type;
    union MessageData
    {
        struct Request request;
        struct Response response;
    } data;
};



#define door_semaphore "door_semaphore"
#define reception_in_semaphore "reception_in_semaphore"
#define reception_out_semaphore "reception_out_semaphore"
#define memory "memory"
struct State
{
    sem_t* door;
    sem_t* reception_in;
    sem_t* reception_out;
    struct Message* shared_memory;
};