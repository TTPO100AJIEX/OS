#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../protocol.h"

#include <time.h>
#include <sys/time.h>
int print_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) return -1; // Get current time up to microseconds
    char buffer[16];
    strftime(buffer, 16, "%H:%M:%S", localtime(&tv.tv_sec)); // Parse current time into nice string
    printf("[%s.%03d.%03d] ", buffer, (int)(tv.tv_usec / 1000), (int)(tv.tv_usec % 1000)); // Print current time up to microseconds
    return 0;
}

int multicast_receiver = -1;
void stop(__attribute__ ((unused)) int signal)
{
    if (close(multicast_receiver) == -1) { perror("Failed to close the socket"); exit(1); }
    exit(0);
}

int main(int argc, char** argv) // <IP> <Port>
{
    // Check and parse the command line arguments
    if (argc < 3) { printf("Not enough command line arguments specified: <IP> <Port>\n"); return 1; }

    // Construct address structure
    struct sockaddr_in multicast_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[2])), .sin_addr = { .s_addr = htonl(INADDR_ANY) } };

    setbuf(stdout, NULL); // Remove the buffering of stdout
    signal(SIGINT, stop); // Register SIGINT handler

    // Create the socket
    multicast_receiver = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (multicast_receiver == -1) { perror("Failed to create a socket"); return 1; }
    print_time(); printf("Created the socket\n");
    
    // Set socket options to allow multiple loggers at once
    int socket_flag = 1;
    if (setsockopt(multicast_receiver, SOL_SOCKET, SO_REUSEADDR, (void*)(&socket_flag), sizeof(socket_flag)) == -1) { perror("Failed to setup the socket"); raise(SIGINT); }

    // Bind to the multicast address
    if (bind(multicast_receiver, (struct sockaddr *)(&multicast_address), sizeof(multicast_address)) == -1) { perror("Failed to bind the socket"); raise(SIGINT); }
    
    // Join the multicast
    struct ip_mreq multicast_request = { .imr_multiaddr = { .s_addr = inet_addr(argv[1]) }, .imr_interface = { .s_addr = htonl(INADDR_ANY) } };
    if (setsockopt(multicast_receiver, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) == -1) { perror("Failed to setup the multicast"); raise(SIGINT); }

    while (true)
    {
        // Receive a message from the server
        char message[LOG_MAX_MESSAGE_SIZE + 1];
        int size = recvfrom(multicast_receiver, message, LOG_MAX_MESSAGE_SIZE, 0, NULL, 0);
        if (size <= 0) { perror("Failed to receive a message"); raise(SIGINT); }
        message[size] = '\0'; // Add the string terminator

        // Print the received message
        printf("%s", message);

        // Check if it is an end message
        if (strcmp(message, LOG_END_MESSAGE) == 0) raise(SIGINT);
    }
}