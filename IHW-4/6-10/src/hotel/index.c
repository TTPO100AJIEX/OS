#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "./rooms/rooms.h"

#include <time.h>
#include <sys/time.h>
static int print_time()
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

    // Initialize the rooms
    init_rooms();

    // Create the socket
    server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == -1) { perror("Failed to create a socket"); raise(SIGINT); }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); raise(SIGINT); }

    // Log that everything is ok
    print_time(); printf("Started the server\n"); print_rooms();

    size_t request_id = 0;
    while (true)
    {
        // Receive a request
        request_id++;
        struct Request request;
        struct sockaddr_in client;
        socklen_t client_struct_length = sizeof(client);
        if (recvfrom(server, &request, sizeof(request), 0, (struct sockaddr *)(&client), &client_struct_length) != sizeof(request)) { perror("Failed to receive a request"); raise(SIGINT); }

        switch (request.type)
        {
            case COME_REQUEST:
            {
                // Log the request
                print_time();
                if (request.data.come.gender != GENDER_MALE && request.data.come.gender != GENDER_FEMALE) { printf("Received an invalid come request from %s:%d\n", inet_ntoa(client.sin_addr), client.sin_port); break; }
                printf("Assigned id %zu to a come request from %s:%d (%c)\n", request_id, inet_ntoa(client.sin_addr), client.sin_port, request.data.come.gender == GENDER_MALE ? 'm' : 'f');

                // Find a room for the visitor
                const struct Room* room = take_room(request_id, request.data.come.gender, request.data.come.stay_time, client);

                // Log the room
                print_time();
                if (room)
                {
                    printf("Registered the visitor %zu into the room %zu\n", request_id, room->id);
                    print_rooms();
                }
                else
                {
                    printf("Failed to register the visitor %zu\n", request_id);
                }

                // Send the response to the visitor
                struct Response res = { .type = COME_RESPONSE, .data = { .come = { .id = request_id, .room = room ? room->id : 0 } } };
                if (sendto(server, &res, sizeof(res), 0, (struct sockaddr *)(&client), sizeof(client)) != sizeof(res)) perror("Failed to send a response");

                break;
            }
            case LEAVE_REQUEST:
            {
                // TODO
                break;
            }
            default: { }
        }
    }

   /*
                // Log the visitor
                if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); kill(rooms.owner, SIGINT); }
                if (gender == MALE && log_message(&logger, "Received gender MALE from the visitor") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (gender == FEMALE && log_message(&logger, "Received gender FEMALE from the visitor") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); kill(rooms.owner, SIGINT); }
        

                // Wait for the client to close the connection - leave the hotel
                char tmp; recv(client, &tmp, sizeof(tmp), 0);

                // Free the room and log it
                if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); kill(rooms.owner, SIGINT); }
                room = free_room(&rooms);
                if (room == -1)
                {
                    if (log_message(&logger, "Visitor left") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                }
                else
                {
                    if (log_message(&logger, "Visitor left room ") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_integer(&logger, room) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_layout(&logger, &rooms) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                }
                if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); kill(rooms.owner, SIGINT); }

                raise(SIGINT);
                break;
            }
            default:
            {
                if (close(client) == -1) { perror("Failed to close the client"); raise(SIGINT); }
            }
        }
    }
    */
}