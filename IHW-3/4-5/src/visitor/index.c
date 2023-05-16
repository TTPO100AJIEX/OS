#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"
#include "log/log.h"

void stop(__attribute__ ((unused)) int signal) { }

int main(int argc, char** argv) // <IP> <Port>
{
    // Check and parse the command line arguments
    if (argc < 5) { printf("Not enough command line arguments specified: <IP> <Port> <Gender (m/f)> <Time>\n"); return 1; }

    enum Gender gender = NONE;
    if (argv[3][0] == 'm') gender = MALE;
    if (argv[3][0] == 'f') gender = FEMALE;
    if (gender == NONE) { printf("Invalid gender specified\n"); return 1; }

    unsigned int time = atoi(argv[4]);
    if (time == 0) { printf("Invalid time specified\n"); return 1; }
    
    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1); // Signals must interrupt all system calls
    signal(SIGINT, stop); // Register an empty handler for the change to take effect

    // Create the socket
    int client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) { perror("Failed to create a socket"); return 1; }
    // Connect to the server
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[2])), .sin_addr = { .s_addr = inet_addr(argv[1]) } };
    if (connect(client, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to connnect to the server"); close(client); return 1; }
    log("Connected to the server\n");

    // Send the request to get a room
    if (send(client, &gender, sizeof(gender), 0) != sizeof(gender)) { perror("Failed to send the gender"); close(client); return 1; }
    log("Sent gender %s to the server\n", gender == MALE ? "male" : "female");

    // Receive the response
    enum ComeStatus status;
    if (recv(client, &status, sizeof(status), 0) != sizeof(status)) { perror("Failed to receive the status"); close(client); return 1; }
    log("Received status %s from the server\n", status == COME_OK ? "OK" : "SORRY");
    
    // Parse the response
    if (status == COME_OK)
    {
        log("Started sleeping (time: %u)\n", time);
        sleep(time); // Sleep for the specified time
        log("Stopped sleeping\n");
    }

    if (close(client) == -1) { perror("Failed to close the socket"); return 1; }
    return 0;
}