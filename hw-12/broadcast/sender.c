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
    // Input the broadcast IP
    char broadcast_ip[16];
    printf("Please enter the broadcast IP: ");
    fgets(broadcast_ip, 15, stdin);

    // Input the broadcast port
    uint16_t broadcast_port = 5000;
    printf("Please enter the broadcast port: ");
    scanf("%"SCNd16, &broadcast_port);

    // Create the socket
    int connection = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connection == -1) { perror("Failed to create a socket"); return 1; }

    // Set socket to allow broadcast
    int broadcastPermission = 1;
    if (setsockopt(connection, SOL_SOCKET, SO_BROADCAST, (void*)(&broadcastPermission), sizeof(broadcastPermission)) == -1) { perror("Failed to setup the broadcast"); close(connection); return 1; }

    // Construct address structure
    struct sockaddr_in broadcast_address = {
        .sin_family = AF_INET,
        .sin_port = htons(broadcast_port),
        .sin_addr = { .s_addr = inet_addr(broadcast_ip) }
    };

    char message[MAX_MESSAGE_SIZE + 1]; // Initialize the message object
    fgets(message, MAX_MESSAGE_SIZE, stdin); // Skip the remaining symbols
    while (true)
    {
        // Input the message
        printf("Enter the message up to %d symbols: ", MAX_MESSAGE_SIZE);
        fgets(message, MAX_MESSAGE_SIZE, stdin);
        // Add the null terminator
        if (message[strlen(message) - 1] == '\n') message[strlen(message) - 1] = '\0';
        else message[strlen(message)] = '\0';

        // Send the message to the server
        if (sendto(connection, message, strlen(message), 0, (struct sockaddr *)(&broadcast_address), sizeof(broadcast_address)) != strlen(message)) { perror("Failed to broadcast a message"); close(connection); return 1; }
        
        // Check if it was an end message
        if (strcmp(message, END_MESSAGE) == 0) break;
    }

    // Close the connnection and stop
    if (close(connection) == -1) { perror("Failed to close the connection"); return 1; }
    return 0;
}