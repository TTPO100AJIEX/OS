#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "log/log.h"
#include "rooms/rooms.h"

struct Logger logger;
struct Rooms rooms;
int socket_to_close_on_stop;
void stop(__attribute__ ((unused)) int signal)
{
    if (close(socket_to_close_on_stop) == -1) { perror("Failed to close the socket"); delete_rooms(&rooms); delete_logger(&logger); exit(1); }
    if (delete_rooms(&rooms) == -1) { perror("Failed to delete the logger"); delete_logger(&logger); exit(1); }
    if (delete_logger(&logger)) { perror("Failed to delete the rooms"); exit(1); }
    exit(0);
}

int main(int argc, char** argv) // <Port>
{
    // Check the command line arguments
    if (argc < 2) { printf("Not enough command line arguments specified: <Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    signal(SIGINT, stop); // Register SIGINT handler

    // Initialize the logger
    logger = initialize_logger();
    if (!logger.ok) { perror("Failed to initialize the logger"); raise(SIGINT); }

    // Initialize the rooms
    rooms = initialize_rooms("/hotel_rooms_memory", "/hotel_rooms_semaphore", 10, 15);
    if (!rooms.ok) { perror("Failed to initialize the rooms"); raise(SIGINT); }

    // Print the initial layout
    if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); raise(SIGINT); }
    if (log_message(&logger, "Initialized the rooms\n") == -1) { perror("Failed to log a message"); raise(SIGINT); }
    if (log_layout(&logger, &rooms) == -1) { perror("Failed to log a message"); raise(SIGINT); }
    if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); raise(SIGINT); }

    // Create the socket
    int server = socket_to_close_on_stop = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) { perror("Failed to create a socket"); raise(SIGINT); }
    // Set socket options to allow easy disconnect
    int socket_flag = 1;
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (void*)(&socket_flag), sizeof(socket_flag)) == -1) { perror("Failed to setup the socket"); raise(SIGINT); }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); raise(SIGINT); }
    // Start listening for incoming connections
    if (listen(server, 5) == 1) { perror("Failed to listen for incoming connections"); raise(SIGINT); }
    
    // Print the log
    if (lock_rooms(&rooms) == -1) { perror("Failed to lock the rooms"); raise(SIGINT); }
    if (log_message(&logger, "Started the server\n") == -1) { perror("Failed to log a message"); raise(SIGINT); }
    if (unlock_rooms(&rooms) == -1) { perror("Failed to unlock the rooms"); raise(SIGINT); }
    
    while (true)
    {
        // Receive the connection from the client
        int client = accept(server, NULL, 0);
        if (client == -1) { perror("Failed to accept a connection"); raise(SIGINT); }

        pid_t process = fork(); // Create a child process that will handle the client
        if (process == -1) { perror("Failed to fork"); close(client); raise(SIGINT); } // If something went wrong, stop the program
        if (process != 0) { close(client); continue; } // Parent process; the client is not needed in the parent process
        // Child process
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
    }
}