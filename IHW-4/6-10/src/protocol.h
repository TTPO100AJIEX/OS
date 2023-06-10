#pragma once

#include <stddef.h>

enum Gender { GENDER_NONE = 0, GENDER_MALE = 1, GENDER_FEMALE = 2 }; // Genders of visitors



// Request structure
enum RequestType { COME_REQUEST = 0, LEAVE_REQUEST = 1 };

struct ComeRequest
{
    enum Gender gender;
    unsigned int stay_time;
};
struct LeaveRequest
{
    size_t id;
    size_t room;
};

struct Request
{
    enum RequestType type;
    union RequestData
    {
        struct ComeRequest come;
        struct LeaveRequest leave;
    } data;
};



// Response structure
enum ResponseType { COME_RESPONSE = 0, LEAVE_RESPONSE = 1 };

struct ComeResponse
{
    size_t id;
    size_t room;
};
struct LeaveResponse
{
};

struct Response
{
    enum ResponseType type;
    union ResponseData
    {
        struct ComeResponse come;
        struct LeaveResponse leave;
    } data;
};