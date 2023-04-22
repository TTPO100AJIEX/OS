#pragma once

#include <sys/types.h>

enum Gender { NONE, MALE, FEMALE };


// Request structures
struct ComeRequest
{
    enum Gender gender;
    pid_t id;
};
struct LeaveRequest
{
    pid_t id;
};

// Response structures
struct ComeResponse
{
    enum ComeStatus { COME_OK, COME_SORRY } status;
};
struct LeaveResponse
{
    enum LeaveStatus { LEAVE_OK } status;
};

// Message structure
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