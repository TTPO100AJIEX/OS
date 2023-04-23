#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "common.h"

int main(void)
{
    setbuf(stdout, NULL); // Remove the buffering of stdout

    // Create the socket
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) { perror("Failed to create a socket"); return 1; }

    // Input the port
    uint16_t port;
    printf("Please enter a port to use: ");
    scanf("%"SCNd16, &port);

    // Bind the socket
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    };
    if (bind(server, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to bind the socket"); close(server); return 1; }

    // Start listening for incoming connections
    if (listen(server, 5) == 1) { perror("Failed to listen for incoming connections"); close(server); return 1; }

    // Receive the connection from the second client
    int client2 = accept(server, NULL, 0);
    if (client2 == -1) { perror("Failed to accept a connection from the second client"); close(server); return 1; }

    // Receive the connection from the first client
    int client1 = accept(server, NULL, 0);
    if (client1 == -1) { perror("Failed to accept a connection from the first client"); close(client2); close(server); return 1; }

    while(true)
    {
        // Get the message from the first client
        char message[MAX_MESSAGE_SIZE + 1];
        int messageSize = recv(client1, message, MAX_MESSAGE_SIZE, 0);
        if (messageSize <= 0) { perror("Failed to receive a message from the first client"); close(client1); close(client2); close(server); return 1; }
        message[messageSize] = '\0'; // Add the string terminator

        // Print the received message because why not
        printf("Received %s\n", message);

        // Send the message to the second client
        if (send(client2, message, messageSize, 0) != messageSize) { perror("Failed to send the message to the second client"); close(client1); close(client2); close(server); return 1; }
    
        // Check if it was an end message
        if (strcmp(message, END_MESSAGE) == 0) break;
    }

    // Close everything and stop
    close(client1);
    close(client2);
    close(server);
    return 0;
}