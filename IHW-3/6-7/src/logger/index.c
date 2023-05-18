#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"

void stop(__attribute__ ((unused)) int signal) { }

int main(int argc, char** argv) // <IP> <Port>
{
    // Check and parse the command line arguments
    if (argc < 3) { printf("Not enough command line arguments specified: <IP> <Port>\n"); return 1; }
    
    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1); // Signals must interrupt all system calls
    signal(SIGINT, stop); // Register an empty handler for the change to take effect

    // Create the socket
    int client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) { perror("Failed to create a socket"); return 1; }
    // Connect to the server
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[2])), .sin_addr = { .s_addr = inet_addr(argv[1]) } };
    if (connect(client, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to connnect to the server"); close(client); return 1; }
    printf("Connected to the server\n");

    // Send the request to specify who I am
    enum ClientType client_type = LOGGER;
    if (send(client, &client_type, sizeof(client_type), 0) != sizeof(client_type)) { perror("Failed to send the request with the client_type"); close(client); return 1; }
    printf("Sent client_type to the server\n");

    char message[1024];
    while (recv(client, message, sizeof(message), 0) > 0) printf(message);

    if (close(client) == -1) { perror("Failed to close the socket"); return 1; }

    return 0;
}