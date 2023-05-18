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

void stop(__attribute__ ((unused)) int signal) { }

int main(int argc, char** argv) // <Port>
{
    // Check the command line arguments
    if (argc < 2) { printf("Not enough command line arguments specified: <Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1); // Signals must interrupt all system calls
    signal(SIGINT, stop); // Register an empty handler for the change to take effect
    if (initialize_log("/log_semaphore") == -1) { perror("Failed to initialize the logging"); return 1; } // Initialize the logging

    // Initialize the rooms
    struct Rooms rooms = initialize_rooms("/rooms_rooms", "/rooms_rooms_sync");
    if (!rooms.ok) { perror("Failed to initialize to rooms"); destroy_log(); return 1; }
    char* rooms_layout = get_rooms_layout(&rooms);
    log("Initialized the rooms\n%s\n", rooms_layout);
    free(rooms_layout);

    // Create the socket
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) { perror("Failed to create a socket"); destroy_rooms(&rooms); destroy_log(); return 1; }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); close(server); destroy_rooms(&rooms); destroy_log(); return 1; }
    // Start listening for incoming connections
    if (listen(server, 5) == 1) { perror("Failed to listen for incoming connections"); close(server); destroy_rooms(&rooms); destroy_log(); return 1; }
    log("Started the server\n");

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
        if (close(server) == -1) { perror("Failed to close the server"); close_rooms(&rooms); close(client); close_log(); return 1; }
        log("Accepted a connection (pid: %d)\n", getpid());

        // Receive visitor's gender
        enum Gender gender = NONE;
        if (recv(client, &gender, sizeof(gender), 0) == sizeof(gender)) // Otherwise, something went wrong and the request should be declined
        {
            log("Received gender %s from the visitor (pid: %d)\n", gender == MALE ? "MALE" : "FEMALE", getpid());
            
            // Find a room for the visitor
            int room = take_room(&rooms, gender);
            if (room == -1) { log("Failed to register (pid: %d)\n", getpid()); }
            else { char* layout = get_rooms_layout(&rooms); log("Registered into the room %d (pid: %d)\n%s\n", room, getpid(), layout); free(layout); }
            
            // Sleep for more realistic interaction
            struct timespec to_sleep = { .tv_sec = 0, .tv_nsec = 5e8 };
            nanosleep(&to_sleep, NULL);

            // Send the status to the visitor
            enum ComeStatus status = (room == -1 ? COME_SORRY : COME_OK);
            if (send(client, &status, sizeof(status), 0) == sizeof(status))
            {
                // Wait for the client to close the connection - leave the hotel
                char tmp; recv(client, &tmp, sizeof(tmp), 0);
                // Free the room
                int room = free_room(&rooms);
                if (room == -1) { log("Visitor left (pid: %d)\n", getpid()); }
                else { char* layout = get_rooms_layout(&rooms); log("Visitor left room %d (pid: %d)\n%s\n", room, getpid(), layout); free(layout); }
            }
        }

        // Close everything
        if (close_rooms(&rooms) == -1) { perror("Failed to close the rooms"); close(client); close_log(); return 1; }
        if (close(client) == -1) { perror("Failed to close the client"); close_log(); return 1; }
        log("Closed the connection (pid: %d)\n", getpid());
        if (close_log() == -1) { perror("Failed to close the logger"); return 1; }
        return 0;
    }

    // Delete everything
    if (destroy_rooms(&rooms) == -1) { perror("Failed to destroy the rooms"); close(server); destroy_log(); return 1; }
    if (close(server) == -1) { perror("Failed to close the server"); destroy_log(); return 1; }
    log("Stopped the server\n");
    if (destroy_log() == -1) { perror("Failed to destroy the logger"); return 1; }
    return 0;
}