#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"

void stop(__attribute__ ((unused)) int signal) { }

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

int main(int argc, char** argv) // <IP> <Port>
{
    // Check and parse the command line arguments
    if (argc < 3) { printf("Not enough command line arguments specified: <IP> <Port>\n"); return 1; }
    
    setbuf(stdout, NULL); // Remove the buffering of stdout
    // Register an empty handler for SIGINT to make it interrupt all system calls
    struct sigaction sigint_settings = { .sa_handler = stop, .sa_flags = 0 };
    sigaction(SIGINT, &sigint_settings, NULL);

    // Create the socket
    int client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) { perror("Failed to create a socket"); return 1; }
    // Connect to the server
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(atoi(argv[2])), .sin_addr = { .s_addr = inet_addr(argv[1]) } };
    if (connect(client, (struct sockaddr *)(&server_address), sizeof(server_address)) == -1) { perror("Failed to connnect to the server"); goto stop_client; }
    print_time();
    printf("Connected to the server\n");

    // Send the request to specify who I am
    enum ClientType client_type = LOGGER;
    if (send(client, &client_type, sizeof(client_type), 0) != sizeof(client_type)) { perror("Failed to send the request with the client_type"); goto stop_client; }
    print_time();
    printf("Sent client_type to the server\n");

    char message[1024]; // A buffer that is definitely enough
    int size;
    while ((size = recv(client, message, sizeof(message) - 1, 0)) > 0)
    {
        message[size] = '\0'; // Add the null terminator
        printf("%s", message); // Read while there is something to read and print it to the console; recv will fail when the server is turned off
    }

stop_client:
    if (close(client) == -1) { perror("Failed to close the socket"); return 1; }
    return 0;
}