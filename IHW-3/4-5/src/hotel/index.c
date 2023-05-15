#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "rooms/rooms.h"

void stop(__attribute__ ((unused)) int signal) { }

int main(int argc, char** argv) // <Port>
{
    // Check command line arguments
    if (argc < 2) { printf("Not enough command line arguments specified: <Port>\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1); // Signals must interrupt all system calls
    signal(SIGINT, stop); // Register an empty handler for the change to take effect

    // Initialize the rooms
    struct Rooms rooms = initialize_rooms("/rooms_rooms", "/rooms_rooms_sync");
    if (!rooms.ok) { perror("Failed to initialize to rooms"); return 1; }

    // Create the socket
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) { perror("Failed to create a socket"); destroy_rooms(&rooms); return 1; }
    // Bind the socket
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[1])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); close(server); destroy_rooms(&rooms); return 1; }
    // Start listening for incoming connections
    if (listen(server, 5) == 1) { perror("Failed to listen for incoming connections"); close(server); destroy_rooms(&rooms); return 1; }

    // Delete everything
    if (destroy_rooms(&rooms) == -1) { perror("Failed to destroy the rooms"); close(server); return 1; }
    if (close(server) == -1) { perror("Failed to close the server"); return 1; }
    return 0;

    /*
    while (true)
    {
        // Receive the connection from the client
        int client = accept(server, NULL, 0);
        if (client == -1) break; // Stop the hotel if something went wrong (usually - a signal interruption)
        pid_t process = fork(); // Create a child process that will handle the client
        if (process == -1) { perror("Failed to fork"); break; } // if something went wrong, stop the program
        if (process == 0)
        {
            // Visitor handler
            close(server);

            enum Gender gender = NONE;
            recv(client, &gender, sizeof(gender), 0);
            printf("%d", gender);

            enum ComeStatus status = COME_OK;
            send(client, &status, sizeof(status), 0);

            char tmp[10];
            recv(client, tmp, sizeof(tmp), 0);
            printf("!");

            close_semaphore(rooms_sync);
            close(client);
            return 0;
        }
        else
        {
            close(client);
        }
    }*/
}