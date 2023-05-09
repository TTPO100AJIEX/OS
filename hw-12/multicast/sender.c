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
    // Input the multicast IP
    char multicast_ip[16];
    printf("Please enter the multicast IP: ");
    fgets(multicast_ip, 15, stdin);

    // Input the multicast port
    uint16_t multicast_port = 5000;
    printf("Please enter the multicast port: ");
    scanf("%"SCNd16, &multicast_port);

    // Create the socket
    int connection = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (connection == -1) { perror("Failed to create a socket"); return 1; }

    // Set TTL of multicast packets
    int multicastTTL = 1;
    if (setsockopt(connection, IPPROTO_IP, IP_MULTICAST_TTL, &multicastTTL, sizeof(multicastTTL)) == -1) { perror("Failed to setup the multicast TTL"); close(connection); return 1; }

    // Construct address structure
    struct sockaddr_in multicast_address = {
        .sin_family = AF_INET,
        .sin_port = htons(multicast_port),
        .sin_addr = { .s_addr = inet_addr(multicast_ip) }
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

        // Send the message to the receivers
        if (sendto(connection, message, strlen(message), 0, (struct sockaddr *)(&multicast_address), sizeof(multicast_address)) != strlen(message)) { perror("Failed to send a message"); close(connection); return 1; }
        
        // Check if it was an end message
        if (strcmp(message, END_MESSAGE) == 0) break;
    }

    // Close the connnection and stop
    if (close(connection) == -1) { perror("Failed to close the connection"); return 1; }
    return 0;
}