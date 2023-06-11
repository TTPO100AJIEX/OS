#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../protocol.h"
#include "./rooms/rooms.h"
#include "./log/log.h"
void log_string(const char* string);

int server = -1;
struct sockaddr_in multicast_address;
void stop(__attribute__ ((unused)) int signal)
{
    destroy_rooms(); // Force everyone to leave
    log_string(LOG_END_MESSAGE); // Stop the loggers
    if (close(server) == -1) { perror("Failed to close the socket"); exit(1); }
    exit(0);
}

void log_string(const char* string)
{
    printf("%s", string);
    // Multicast the message
    if (sendto(server, string, strlen(string), 0, (struct sockaddr *)(&multicast_address), sizeof(multicast_address)) != (int)(strlen(string)))
    {
        perror("Failed to multicast a message");
        raise(SIGINT);
    }
}

int main(int argc, char** argv) // <Port> <Multicast-IP> <Multicast-Port>
{
    // Check the command line arguments
    if (argc < 4) { printf("Not enough command line arguments specified: <Port> <Multicast-IP> <Multicast-Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    signal(SIGINT, stop); // Register SIGINT handler
    
    // Construct the multicast address
    multicast_address = (struct sockaddr_in){ .sin_family = AF_INET, .sin_port = htons(atoi(argv[3])), .sin_addr = { .s_addr = inet_addr(argv[2]) } };

    init_rooms(); // Initialize the rooms

    // Create the socket
    server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == -1) { perror("Failed to create a socket"); raise(SIGINT); }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); raise(SIGINT); }

    log_message("Started the server\n"); log_layout(); // Log that everything is ok

    while (true)
    {
        struct RequestWrapper req = receive_request(server); // Receive a request
        switch (req.request.type)
        {
            case COME_REQUEST:
            {
                struct ComeRequest request = req.request.data.come;

                // Log the request
                if (request.gender != GENDER_MALE && request.gender != GENDER_FEMALE) { log_parametric("Received an invalid come request from %s:%d\n", inet_ntoa(req.client.sin_addr), req.client.sin_port); break; }
                log_parametric("Assigned id %zu to a come request from %s:%d (%c)\n", req.id, inet_ntoa(req.client.sin_addr), req.client.sin_port, request.gender == GENDER_MALE ? 'm' : 'f');

                // Find a room for the visitor
                const struct Room* room = take_room(req.id, request.gender, request.stay_time, req.client);

                // Log the room
                if (room) { log_parametric("Registered visitor %zu into the room %zu\n", req.id, room->id); log_layout(); }
                else log_parametric("Failed to register the visitor %zu\n", req.id);

                // Send the response to the visitor
                send_response(server, (struct Response){ .type = COME_RESPONSE, .data = { .come = { .id = req.id, .room = room ? room->id : 0 } } }, req.client);

                break;
            }
            case LEAVE_REQUEST:
            {
                struct LeaveRequest request = req.request.data.leave;

                // Free the room
                const struct Room* room = free_room(request.room, request.id);

                // Log the room
                if (room) { log_parametric("Visitor %zu left the room %zu\n", request.id, room->id); log_layout(); }
                else log_parametric("Visitor %zu left\n", request.id);

                break;
            }
            default: { }
        }
    }
}