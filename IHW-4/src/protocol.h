#pragma once

#include <stddef.h>
#include <sys/socket.h>
#include <arpa/inet.h>


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


// Send utilities
void send_request(int socket, struct Request request, struct sockaddr_in address);
void send_response(int socket, struct Response response, struct sockaddr_in address);

// Receive utilities
struct RequestWrapper
{
    size_t id;
    struct Request request;
    struct sockaddr_in client;
};
struct ResponseWrapper
{
    struct Response response;
    struct sockaddr_in server;
};
struct RequestWrapper receive_request(int socket);
struct ResponseWrapper receive_response(int socket, enum ResponseType type);

// Logger settings
#define LOG_MAX_MESSAGE_SIZE 1024
#define LOG_END_MESSAGE "The End\n"