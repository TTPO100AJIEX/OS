#pragma once

enum Gender { GENDER_NONE = 0, GENDER_MALE = 1, GENDER_FEMALE = 2 }; // Genders of visitors

// Request structure
struct Request
{
    enum RequestType { COME_REQUEST = 0, LEAVE_REQUEST = 1 } type;
    union RequestData
    {
        struct ComeRequest
        {
            enum Gender gender;
            unsigned int time;
        } come_request;

        struct LeaveRequest
        {
            unsigned int id;
            unsigned int room;
        } leave_request;
    } data;
};

// Response structure
struct Response
{
    enum ResponseType { COME_RESPONSE = 0, LEAVE_RESPONSE = 1 } type;
    union ResponseData
    {
        struct ComeResponse
        {
            unsigned int id;
            unsigned int room;
        } come_request;

        struct LeaveResponse
        {
            enum LeaveStatus { LEAVE_OK = 0 } status;
        } leave_request;
    } data;
};