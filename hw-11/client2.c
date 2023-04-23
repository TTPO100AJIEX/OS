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
    int connection = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connection == -1) { perror("Failed to create a socket"); return 1; }

    // Input the IP of the server
    char server_ip[16];
    printf("Please enter the IP of the server: ");
    fgets(server_ip, 15, stdin);

    // Input the port of the server
    uint16_t server_port = 5000;
    printf("Please enter the port of the server: ");
    scanf("%"SCNd16, &server_port);

    // Connect to the server
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(server_port),
        .sin_addr = { .s_addr = inet_addr(server_ip) }
    };
    if (connect(connection, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to connect to the server"); close(connection); return 1; }
    printf("Established connection with the server\n");

    while (true)
    {
        // Receive a message from the server
        char message[MAX_MESSAGE_SIZE + 1];
        int size = recv(connection, message, MAX_MESSAGE_SIZE, 0);
        if (size <= 0) { perror("Failed to get a message from the server"); close(connection); return 1; }
        message[size] = '\0'; // Add the string terminator
        
        // Print the received message because why not
        printf("Received %s\n", message);

        // Check if it is an end message
        if (strcmp(message, END_MESSAGE) == 0) break;
    }

    // Close the connnection and stop
    if (close(connection) == -1) { perror("Failed to close the connection"); return 1; }
    return 0;
}