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

static int server = -1;
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

    // Log the everything is ok
    print_time(); printf("Started the server\n"); print_rooms();

    while (true)
    {
        // Receive a request
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
                printf("Received a come request from %s:%d (%c)\n", inet_ntoa(client.sin_addr), client.sin_port, request.data.come.gender == GENDER_MALE ? 'm' : 'f');

                // Find a room for the visitor
                int room = take_room(request.data.come.gender, request.data.come.stay_time, client.sin_addr, client.sin_port);



                


                int room = take_room(&rooms, gender);
                if (room == -1)
                {
                    if (log_message(&logger, "Failed to register") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                }
                else
                {
                    if (log_message(&logger, "Registered into the room ") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_integer(&logger, room) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_layout(&logger, &rooms) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                }
                if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); kill(rooms.owner, SIGINT); }
                
                // Send the status to the visitor
                enum ComeStatus status = (room == -1 ? COME_SORRY : COME_OK);
                if (send(client, &status, sizeof(status), MSG_NOSIGNAL) != sizeof(status)) raise(SIGINT); // Something went wrong and the connection should be stopped


                struct Response res = { .type = COME_RESPONSE, .data = { .come = { .id = 1, .room = 2 } } };
                if (sendto(server, &res, sizeof(res), 0, (struct sockaddr *)(&client), sizeof(client)) != sizeof(res))
                {
                    perror("Failed to send a response");
                }
                break;
            }
            case LEAVE_REQUEST:
            {
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
    /*
    // Initialize the rooms
    rooms = initialize_rooms("/hotel_rooms_memory", "/hotel_rooms_semaphore", 10, 15);
    if (!rooms.ok) { perror("Failed to initialize the rooms"); raise(SIGINT); }

    
    // Print the initial layout
    if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); raise(SIGINT); }
    if (log_message(&logger, "Initialized the rooms\n") == -1) { perror("Failed to log a message"); raise(SIGINT); }
    if (log_layout(&logger, &rooms) == -1) { perror("Failed to log a message"); raise(SIGINT); }
    if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); raise(SIGINT); }

    
    // Print the log
    if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); raise(SIGINT); }
    if (log_message(&logger, "Started the server\n") == -1) { perror("Failed to log a message"); raise(SIGINT); }
    if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); raise(SIGINT); }

    // Change the state of the logger: initially it should be locked
    if (lock_logger(&logger) == -1) { perror("Failed to unlock the logger"); raise(SIGINT); }
    
    while (true)
    {
        // Let someone initiate the logging while parent process is waiting to accept a connection
        if (unlock_logger(&logger) == -1) { perror("Failed to unlock the logger"); raise(SIGINT); }

        // Receive the connection from the client
        int client = accept(server, NULL, 0);
        if (client == -1) { perror("Failed to accept a connection"); raise(SIGINT); }
        
        // Do not let anyone log anything until the request is handled. Logging can only be done while waiting for accept
        if (lock_logger(&logger) == -1) { perror("Failed to unlock the logger"); raise(SIGINT); }

        // Get the type of the client
        enum ClientType client_type = UNKNOWN;
        if (recv(client, &client_type, sizeof(client_type), 0) != sizeof(client_type)) client_type = UNKNOWN; // Something went wrong and the request should be declined
        
        switch (client_type)
        {
            case LOGGER:
            {
                // Just save the client's descriptor
                if (add_log_destination(&logger, client) == -1) { perror("Failed to register a logger"); close(client); raise(SIGINT); }
                break;
            }
            case VISITOR:
            {
                pid_t process = fork(); // Create a child process that will handle the client
                if (process == -1) { perror("Failed to fork"); close(client); raise(SIGINT); } // If something went wrong, stop the program
                if (process != 0) { close(client); break; } // Parent process; the client is not needed in the parent process
                // Child process
                signal(SIGUSR1, SIG_IGN); // remove an unnecessary listener
                socket_to_close_on_stop = client;

                // Close the server as it is not needed in the child process
                if (close(server) == -1) { perror("Failed to close the server"); raise(SIGINT); }
                

                // Log the connection
                if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); kill(rooms.owner, SIGINT); }
                if (log_message(&logger, "Accepted a visitor") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); kill(rooms.owner, SIGINT); }

                // Receive visitor's gender
                enum Gender gender = NONE;
                if (recv(client, &gender, sizeof(gender), 0) != sizeof(gender)) raise(SIGINT); // Something went wrong and the request should be declined

                // Log the visitor
                if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); kill(rooms.owner, SIGINT); }
                if (gender == MALE && log_message(&logger, "Received gender MALE from the visitor") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (gender == FEMALE && log_message(&logger, "Received gender FEMALE from the visitor") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); kill(rooms.owner, SIGINT); }
        
                // Find a room for the visitor and log it
                if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); kill(rooms.owner, SIGINT); }
                int room = take_room(&rooms, gender);
                if (room == -1)
                {
                    if (log_message(&logger, "Failed to register") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                }
                else
                {
                    if (log_message(&logger, "Registered into the room ") == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_integer(&logger, room) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_pid(&logger) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                    if (log_layout(&logger, &rooms) == -1) { perror("Failed to log a message"); kill(rooms.owner, SIGINT); }
                }
                if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); kill(rooms.owner, SIGINT); }
                
                // Send the status to the visitor
                enum ComeStatus status = (room == -1 ? COME_SORRY : COME_OK);
                if (send(client, &status, sizeof(status), MSG_NOSIGNAL) != sizeof(status)) raise(SIGINT); // Something went wrong and the connection should be stopped

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
    }*/
}