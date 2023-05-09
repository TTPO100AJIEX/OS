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

    // Construct address structure
    struct sockaddr_in multicast_address = {
        .sin_family = AF_INET,
        .sin_port = htons(multicast_port),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    };

    // Bind to the multicast port
    if (bind(connection, (struct sockaddr *)(&multicast_address), sizeof(multicast_address)) == -1) { perror("Failed to bind to the port"); close(connection); return 1; }
    
    // Join the multicast
    struct ip_mreq multicast_request = {
        .imr_multiaddr = { .s_addr = inet_addr(multicast_ip) },
        .imr_interface = { .s_addr = htonl(INADDR_ANY) }
    };
    if (setsockopt(connection, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) == -1) { perror("Failed to setup the multicast_request"); close(connection); return 1; }

    while (true)
    {
        // Receive a message from the server
        char message[MAX_MESSAGE_SIZE + 1];
        int size = recvfrom(connection, message, MAX_MESSAGE_SIZE, 0, NULL, 0);
        if (size <= 0) { perror("Failed to receive a message"); close(connection); return 1; }
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