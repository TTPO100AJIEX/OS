#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#include "../protocol.h"
#include "./rooms/rooms.h"

#include <time.h>
#include <sys/time.h>
int print_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1; // Get current time up to microseconds
    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec)); // Parse current time into nice string
    printf("[%s.%03d.%03d] ", buffer, (int)(tv.tv_usec / 1000), (int)(tv.tv_usec % 1000)); // Print current time up to microseconds
    return 0;
}

int server = -1;
void stop(__attribute__ ((unused)) int signal)
{
    destroy_rooms();
    if (close(server) == -1) { perror("Failed to close the socket"); exit(1); }
    exit(0);
}

int main(int argc, char** argv) // <Port>
{
    // Check the command line arguments
    if (argc < 2) { printf("Not enough command line arguments specified: <Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    signal(SIGINT, stop); // Register SIGINT handler

    init_rooms(); // Initialize the rooms

    // Create the socket
    server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == -1) { perror("Failed to create a socket"); raise(SIGINT); }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); raise(SIGINT); }

    print_time(); printf("Started the server\n"); print_rooms(); // Log that everything is ok

    while (true)
    {
        struct RequestWrapper req = receive_request(server); // Receive a request
        switch (req.request.type)
        {
            case COME_REQUEST:
            {
                // Log the request
                print_time();
                if (req.request.data.come.gender != GENDER_MALE && req.request.data.come.gender != GENDER_FEMALE) { printf("Received an invalid come request from %s:%d\n", inet_ntoa(req.client.sin_addr), req.client.sin_port); break; }
                printf("Assigned id %zu to a come request from %s:%d (%c)\n", req.id, inet_ntoa(req.client.sin_addr), req.client.sin_port, req.request.data.come.gender == GENDER_MALE ? 'm' : 'f');

                // Find a room for the visitor
                const struct Room* room = take_room(req.id, req.request.data.come.gender, req.request.data.come.stay_time, req.client);

                // Log the room
                print_time();
                if (room)
                {
                    printf("Registered the visitor %zu into the room %zu\n", req.id, room->id);
                    print_rooms();
                }
                else
                {
                    printf("Failed to register the visitor %zu\n", req.id);
                }

                // Send the response to the visitor
                send_response(server, (struct Response){ .type = COME_RESPONSE, .data = { .come = { .id = req.id, .room = room ? room->id : 0 } } }, req.client);
                break;
            }
            case LEAVE_REQUEST:
            {
                const struct Room* room = free_room(req.request.data.leave.room, req.request.data.leave.id);
                print_time();
                if (room) printf("Visitor %zu left\n", req.request.data.leave.id);
                else printf("Visitor %zu left room %zu\n", req.request.data.leave.id, room->id);
                break;
            }
            default: { }
        }
    }
}