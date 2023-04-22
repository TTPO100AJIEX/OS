#include "visitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include "../log/log.h"

static bool running = true;
static struct State state;
static void stop()
{
    // If the signal has already been received, do nothing
    if (!running) return;
    running = false;
    
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
    log("Visitor %d: I want a room\n", getpid());
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
    log("Visitor %d: started sleeping\n", getpid());
    sleep_milliseconds(time * 1000); // Sleep for the specified time
    log("Visitor %d: stopped sleeping\n", getpid());
    return 0;
}
static int leave_hotel()
{
    log("Visitor %d: I want to leave\n", getpid());
    // Construct the message
    struct Message message = {
        .type = LEAVE_REQUEST,
        .data = { .leave_request = { .id = getpid() }  }
    };
    if (request(state, message) == -1) return -1; // Do the request
    if (get_response(state, &message) == -1) return -1; // Get the response
    log("Visitor %d: left the hotel, OK\n", getpid());
    return 0;
}

int visitor(struct State l_state, enum Gender gender, unsigned int time)
{
    state = l_state;
    signal(SIGINT, stop);

    // Ask the hotel for the room and process the response
    int status = come_to_hotel(gender);
    if (status == -1) { printf("Visitor: failed to call the hotel\n"); stop(); return 1; }
    if (status == 1)
    {
        log("Visitor %d: left the hotel, there were no places for me :(\n", getpid());
        stop();
        return 0;
    }

    if (live(time) != 0) { printf("Visitor: failed to live in the hotel\n"); stop(); return 1; } // Live in the hotel for the specified time
    if (leave_hotel() == -1) { printf("Visitor: failed to leave the hotel\n"); stop(); return 1; } // Tell the hotel that I am leaving
    stop(); // Stop the program correctly
    return 0;
}