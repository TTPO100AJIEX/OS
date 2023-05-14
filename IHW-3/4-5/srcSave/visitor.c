#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include "utils/utils.h"
#include "utils/protocol.h"
#include "log/log.h"

static bool running = true;
static struct State state;
static void stop(int sig)
{
    // If the signal has already been received, do nothing
    if (!running) return;
    running = false;
    
    if (sig == SIGINT)
    {
        // Kill the hotel to stop everything
        pid_t hotel_pid;
        read_memory(state.hotel_pid, &hotel_pid, sizeof(pid_t));
        if (kill(hotel_pid, SIGINT) == -1) perror("Failed to kill the hotel");
    }

    // Close all the utilities and exit
    if (close_state(state) == -1)
    {
        printf("Visitor: failed to close the state\n");
        exit(1);
    }
    exit(0);
}

static int come_to_hotel(enum Gender gender)
{
    log("I want a room\n");
    // Construct the message
    struct Message message = {
        .type = COME_REQUEST,
        .data = { .come_request = { .gender = gender, .id = getpid() }  }
    };
    if (request(state, message) == -1) return -1; // Do the request
    if (get_response(state, &message) == -1) return -1; // Get the response
    return message.data.come_response.status == COME_OK ? 0 : 1; // Check the status and return it
}
static int live(unsigned int time)
{
    log("Started sleeping (time: %u)\n", time);
    sleep_milliseconds(time * 1000); // Sleep for the specified time
    log("Stopped sleeping\n");
    return 0;
}
static int leave_hotel()
{
    log("I want to leave\n");
    // Construct the message
    struct Message message = {
        .type = LEAVE_REQUEST,
        .data = { .leave_request = { .id = getpid() }  }
    };
    if (request(state, message) == -1) return -1; // Do the request
    if (get_response(state, &message) == -1) return -1; // Get the response
    log("Left the hotel, OK\n");
    return 0;
}

int main(int argc, char** argv)
{
    // Parse the input
    if (argc < 3) { printf("Not enough command line arguments specified\n"); return 1; }
    enum Gender gender = NONE;
    if (argv[1][0] == 'm') gender = MALE;
    if (argv[1][0] == 'f') gender = FEMALE;
    if (gender == NONE) { printf("Invalid gender specified\n"); return 1; }
    unsigned int time = atoi(argv[2]);
    if (time == 0) { printf("Invalid time specified\n"); return 1; }

    // Initialize all the needed utilities.
    // As the hotel must have been started before any visitors, all semaphores and shared memory segments have already been created,
    // so this will just connect to those without creating any new ones
    if (init_state(&state) == -1)
    {
        printf("Failed to initialize the state\n");
        close_state(state);
        return 1;
    }
    signal(SIGINT, stop); // Register the signal handler to stop everything
    signal(SIGTERM, stop); // Register the signal handler to stop only this visitor
    log("Started with PID %d\n", getpid());


    // Ask the hotel for the room and process the response
    int status = come_to_hotel(gender);
    if (status == -1) { printf("Visitor: failed to call the hotel\n"); raise(SIGINT); return 1; }
    if (status == 1)
    {
        log("Left the hotel, there were no places for me :(\n");
        raise(SIGTERM);
        return 0;
    }

    if (live(time) != 0) { printf("Failed to live in the hotel\n"); raise(SIGINT); return 1; } // Live in the hotel for the specified time
    if (leave_hotel() == -1) { printf("Failed to leave the hotel\n"); raise(SIGINT); return 1; } // Tell the hotel that I am leaving
    raise(SIGTERM); // Stop the program correctly
    return 0;
}