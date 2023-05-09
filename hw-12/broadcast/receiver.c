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

    // Input the broadcast port
    uint16_t broadcast_port = 5000;
    printf("Please enter the broadcast port: ");
    scanf("%"SCNd16, &broadcast_port);
    
    // Create the socket
    int connection = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connection == -1) { perror("Failed to create a socket"); return 1; }

    // Construct local address structure
    struct sockaddr_in broadcast_address = {
        .sin_family = AF_INET,
        .sin_port = htons(broadcast_port),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    };

    // Bind to the broadcast port
    if (bind(connection, (struct sockaddr *)(&broadcast_address), sizeof(broadcast_address)) == -1) { perror("Failed to bind to the port"); close(connection); return 1; }
    
    while (true)
    {
        // Receive a message from the server
        char message[MAX_MESSAGE_SIZE + 1];
        int size = recvfrom(connection, message, MAX_MESSAGE_SIZE, 0, NULL, 0);
        if (size <= 0) { perror("Failed to get a message from the broadcast"); close(connection); return 1; }
        message[size] = '\0'; // Add the string terminator

        // Print the received message
        printf("Received %s\n", message);

        // Check if it is an end message
        if (strcmp(message, END_MESSAGE) == 0) break;
    }

    // Close the connnection and stop
    if (close(connection) == -1) { perror("Failed to close the connection"); return 1; }
    return 0;
}