#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

enum Gender gender = GENDER_NONE;
unsigned int stay_time = 0;
int client = -1;
struct sockaddr_in server_address;
size_t id = 0, room = 0;

void request(struct Request request)
{
    if (sendto(client, &request, sizeof(request), 0, (struct sockaddr *)(&server_address), sizeof(server_address)) != sizeof(request))
    {
        perror("Failed to send a request");
        raise(SIGINT);
    }
}
struct Response response(enum ResponseType type)
{
    struct Response response;
    if (recvfrom(client, &response, sizeof(response), 0, NULL, NULL) != sizeof(response))
    {
        perror("Failed to receive a response");
        raise(SIGINT);
    }
    if (response.type != type)
    {
        printf("Received an invalid response (expected: %d, received: %d)\n", type, response.type);
        raise(SIGINT);
    }
    return response;
}

void stop(__attribute__ ((unused)) int signal)
{
    if (room != 0)
    {
        // Send a leave request
        request((struct Request){ .type = LEAVE_REQUEST, .data = { .leave = { .id = id, .room = room } } });
        print_time(); printf("Left the hotel\n");
        // There is no need to wait for the response
    }   
    if (close(client) == -1) { perror("Failed to close the socket"); exit(1); }
    exit(0);
}

int main(int argc, char** argv) // <IP> <Port> <Gender (m/f)> <Time>
{
    // Check and parse the command line arguments
    if (argc < 5) { printf("Not enough command line arguments specified: <IP> <Port> <Gender (m/f)> <Time>\n"); return 1; }

    server_address = (struct sockaddr_in){ .sin_family = AF_INET, .sin_port = htons(atoi(argv[2])), .sin_addr = { .s_addr = inet_addr(argv[1]) } };
    
    gender = GENDER_NONE;
    if (argv[3][0] == 'm') gender = GENDER_MALE;
    if (argv[3][0] == 'f') gender = GENDER_FEMALE;
    if (gender == GENDER_NONE) { printf("Invalid gender specified\n"); return 1; }

    stay_time = atoi(argv[4]);
    if (stay_time == 0) { printf("Invalid time specified\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    signal(SIGINT, stop); // Register SIGINT handler

    // Create the socket
    client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == -1) { perror("Failed to create a socket"); return 1; }
    print_time(); printf("Created the socket\n");

    // Request a room
    request((struct Request){ .type = COME_REQUEST, .data = { .come = { .gender = gender, .stay_time = stay_time } } });
    print_time(); printf("Sent gender %s, stay_time %d to the server\n", gender == GENDER_MALE ? "male" : "female", stay_time);

    // Wait for the response
    struct Response res = response(COME_RESPONSE);
    print_time(); printf("Assigned id %zu and received room %zu from the server\n", res.data.come.id, res.data.come.room);
    id = res.data.come.id;
    room = res.data.come.room;

    // Parse the response
    if (room == 0)
    {
        print_time();
        printf("Left the hotel, there were no places for me :(\n");
    }
    else
    {
        pid_t process = fork(); // Create a child process to listen for urgent leave requests
        if (process == -1) { perror("Failed to fork"); raise(SIGINT); } // If something went wrong, stop the program
        if (process != 0)
        {
            // Parent process
            print_time(); printf("Started sleeping (time: %u)\n", stay_time);
            sleep(stay_time); // Sleep for the specified time
            print_time(); printf("Stopped sleeping\n");
        }
        else
        {
            // Child process
            room = 0;
            response(LEAVE_RESPONSE);
            print_time(); printf("Forced to leave\n");
            kill(getppid(), SIGINT);
            raise(SIGINT);
            return 0;
        }
    }

    // Stop the program
    raise(SIGINT);
}