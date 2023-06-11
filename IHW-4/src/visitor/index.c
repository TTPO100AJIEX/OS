#include <stdio.h>
#include <stdlib.h>
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


int client = -1;
size_t id = 0, room = 0;
struct sockaddr_in server_address;
void stop(__attribute__ ((unused)) int signal)
{
    if (room)
    {
        // Send a leave request
        send_request(client, (struct Request){ .type = LEAVE_REQUEST, .data = { .leave = { .id = id, .room = room } } }, server_address);
        print_time(); printf("Left the hotel\n");
    }   
    if (close(client) == -1) { perror("Failed to close the socket"); exit(1); }
    exit(0);
}

int main(int argc, char** argv) // <IP> <Port> <Gender (m/f)> <Time>
{
    // Check and parse the command line arguments
    if (argc < 5) { printf("Not enough command line arguments specified: <IP> <Port> <Gender (m/f)> <Time>\n"); return 1; }

    server_address = (struct sockaddr_in){ .sin_family = AF_INET, .sin_port = htons(atoi(argv[2])), .sin_addr = { .s_addr = inet_addr(argv[1]) } };
    
    enum Gender gender = GENDER_NONE;
    if (argv[3][0] == 'm') gender = GENDER_MALE;
    if (argv[3][0] == 'f') gender = GENDER_FEMALE;
    if (gender == GENDER_NONE) { printf("Invalid gender specified\n"); return 1; }

    unsigned int stay_time = atoi(argv[4]);
    if (stay_time == 0) { printf("Invalid time specified\n"); return 1; }

    setbuf(stdout, NULL); // Remove the buffering of stdout
    signal(SIGINT, stop); // Register SIGINT handler

    // Create the socket
    client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == -1) { perror("Failed to create a socket"); return 1; }
    print_time(); printf("Created the socket\n");

    // Request a room
    send_request(client, (struct Request){ .type = COME_REQUEST, .data = { .come = { .gender = gender, .stay_time = stay_time } } }, server_address);
    print_time(); printf("Sent gender %s, stay_time %d to the hotel\n", gender == GENDER_MALE ? "male" : "female", stay_time);

    // Wait for the response
    struct ResponseWrapper res = receive_response(client, COME_RESPONSE);
    id = res.response.data.come.id; room = res.response.data.come.room;
    print_time(); printf("Assigned id %zu and received room %zu from the hotel\n", id, room);

    // Parse the response
    if (room == 0)
    {
        print_time(); printf("Left the hotel, there were no places for me :(\n");
        raise(SIGINT); // Stop the program
    }

    // The program needs to sleep, but may be interrupted by the hotel with a message
    pid_t child = fork(); // Create a child process to listen for urgent leave requests
    if (child == -1) { perror("Failed to fork"); raise(SIGINT); } // If something went wrong, stop the program
    if (child != 0)
    {
        // Parent process
        print_time(); printf("Started sleeping (time: %u)\n", stay_time);
        sleep(stay_time); // Sleep for the specified time
        print_time(); printf("Stopped sleeping\n");
        kill(child, SIGINT); // Stop the child process
        raise(SIGINT); // Stop the program
    }
    else
    {
        // Child process
        room = 0; // To avoid sending multiple leave requests on SIGINT
        receive_response(client, LEAVE_RESPONSE); // Wait for a leave message
        print_time(); printf("I was forced to leave ;(\n");
        kill(getppid(), SIGINT); // Stop the parent process
        raise(SIGINT); // Stop the current process for compatibility
    }
}