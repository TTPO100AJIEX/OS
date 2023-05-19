#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "rooms/rooms.h"
#include "log/log.h"

struct Logger logger;
void print_logs(__attribute__ ((unused)) int signal)
{

}
void stop(__attribute__ ((unused)) int signal) { }

int main(int argc, char** argv) // <Port>
{
    // Check the command line arguments
    if (argc < 2) { printf("Not enough command line arguments specified: <Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1); // Signals must interrupt all system calls
    signal(SIGINT, stop); // Register an empty handler for the change to take effect

    // Initialize the logger
    struct Logger logger = initialize_log("/log_semaphore", "/log_memory", );
    if (!logger.ok) { perror("Failed to initialize the logger"); return 1; }
    signal(SIGUSR1, print_logs); // Register a signal that sends the logs








    
    

    // Initialize the rooms
    struct Rooms rooms = initialize_rooms("/rooms_rooms", "/rooms_rooms_sync");
    if (!rooms.ok) { perror("Failed to initialize to rooms"); destroy_log(&logger); return 1; }
    if (lock_log(&logger) == -1) perror("Failed to lock the logger");
    log_message(&logger, "Initialized the rooms\n");
    log_layout(&logger, &rooms);
    if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");

    // Create the socket
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) { perror("Failed to create a socket"); destroy_rooms(&rooms); destroy_log(&logger); return 1; }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); close(server); destroy_rooms(&rooms); destroy_log(&logger); return 1; }
    // Start listening for incoming connections
    if (listen(server, 5) == 1) { perror("Failed to listen for incoming connections"); close(server); destroy_rooms(&rooms); destroy_log(&logger); return 1; }
    if (lock_log(&logger) == -1) perror("Failed to lock the logger");
    log_message(&logger, "Started the server\n");
    if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");

    while (true)
    {
        // Receive the connection from the client
        int client = accept(server, NULL, 0);
        if (client == -1) break; // Stop the hotel if something went wrong (usually - a signal interruption)

        pid_t process = fork(); // Create a child process that will handle the client
        if (process == -1) { perror("Failed to fork"); break; } // If something went wrong, stop the program
        if (process != 0) { close(client); continue; } // Parent process; the client is not needed in the parent process
        // Child process

        // Close the server as it is not needed in the child process
        if (close(server) == -1) { perror("Failed to close the server"); close_rooms(&rooms); close(client); close_log(&logger); return 1; }
        if (lock_log(&logger) == -1) perror("Failed to lock the logger");
        log_message(&logger, "Accepted a connection");
        log_pid(&logger);
        if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");


        enum ClientType client_type = UNKNOWN;
        if (recv(client, &client_type, sizeof(client_type), 0) != sizeof(client_type)) { client_type = UNKNOWN; } // If so, something went wrong and the request should be declined
        
        switch (client_type)
        {
            case LOGGER:
            {
                if (add_destination(&logger, client) == -1) perror("Failed to register the logger");
                break;
            }
            case VISITOR:
            {
                // Receive visitor's gender
                enum Gender gender = NONE;
                if (recv(client, &gender, sizeof(gender), 0) != sizeof(gender)) break; // If so, something went wrong and the request should be declined
                if (lock_log(&logger) == -1) perror("Failed to lock the logger");
                if (gender == MALE) log_message(&logger, "Received gender MALE from the visitor");
                else log_message(&logger, "Received gender FEMALE from the visitor");
                log_pid(&logger);
                if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");
                
                // Find a room for the visitor
                int room = take_room(&rooms, gender);
                if (lock_log(&logger) == -1) perror("Failed to lock the logger");
                if (room == -1) { log_message(&logger, "Failed to register"); log_pid(&logger); }
                else { log_message(&logger, "Registered into the room "); log_integer(&logger, room); log_pid(&logger); log_layout(&logger, &rooms); }
                if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");

                // Sleep for more realistic interaction
                struct timespec to_sleep = { .tv_sec = 0, .tv_nsec = 5e8 };
                nanosleep(&to_sleep, NULL);

                // Send the status to the visitor
                enum ComeStatus status = (room == -1 ? COME_SORRY : COME_OK);
                if (send(client, &status, sizeof(status), 0) != sizeof(status)) break;
                // Wait for the client to close the connection - leave the hotel
                char tmp; recv(client, &tmp, sizeof(tmp), 0);
                // Free the room
                room = free_room(&rooms);
                if (lock_log(&logger) == -1) perror("Failed to lock the logger");
                if (room == -1) { log_message(&logger, "Visitor left"); log_pid(&logger); }
                else { log_message(&logger, "Visitor left room "); log_integer(&logger, room); log_pid(&logger); log_layout(&logger, &rooms); }
                if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");
                break;
            }
            default: { }
        }        

        // Close everything
        if (close_rooms(&rooms) == -1) { perror("Failed to close the rooms"); close(client); close_log(&logger); return 1; }
        if (close(client) == -1) { perror("Failed to close the client"); close_log(&logger); return 1; }
        if (lock_log(&logger) == -1) perror("Failed to lock the logger");
        log_message(&logger, "Closed the connection");
        log_pid(&logger);
        if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");
        if (close_log(&logger) == -1) { perror("Failed to close the logger"); return 1; }
        return 0;
    }

    // Delete everything
    if (destroy_rooms(&rooms) == -1) { perror("Failed to destroy the rooms"); close(server); destroy_log(&logger); return 1; }
    if (close(server) == -1) { perror("Failed to close the server"); destroy_log(&logger); return 1; }
    if (lock_log(&logger) == -1) perror("Failed to lock the logger");
    log_message(&logger, "Stopped the server\n");
    if (unlock_log(&logger) == -1) perror("Failed to unlock the logger");
    if (destroy_log(&logger) == -1) { perror("Failed to destroy the logger"); return 1; }
    return 0;
}