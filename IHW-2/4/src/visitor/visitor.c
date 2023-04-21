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
    if (!running) return;
    running = false;
    if (close_state(state) == -1)
    {
        perror("Visitor: failed to close the state");
        exit(1);
    }
    exit(0);
}

static int come_to_hotel(enum Gender gender)
{
    log("Visitor %d: came to the hotel to get a room\n", getpid());
    struct Message message = {
        .type = COME_REQUEST,
        .data = { .come_request = { .gender = gender, .id = getpid() }  }
    };
    if (request(state, message, &message) == -1) return -1;
    return message.data.come_response.status == COME_OK ? 0 : 1;
}
static int live(unsigned int time)
{
    log("Visitor %d: started sleeping\n", getpid());
    sleep_milliseconds(time * 1000);
    log("Visitor %d: stopped sleeping\n", getpid());
    return 0;
}
static int leave_hotel()
{
    log("Visitor %d: came to the hotel to leave\n", getpid());
    struct Message message = {
        .type = LEAVE_REQUEST,
        .data = { .leave_request = { .id = getpid() }  }
    };
    if (request(state, message, &message) == -1) return -1;
    log("Visitor %d: left the hotel, OK\n", getpid());
    return 0;
}

int visitor(struct State l_state, enum Gender gender, unsigned int time)
{
    state = l_state;
    signal(SIGINT, stop);

    int status = come_to_hotel(gender);
    if (status == -1) { perror("Visitor: failed to call the hotel"); return 1; }
    if (status == 1)
    {
        log("Visitor %d: left the hotel, there were no places for me :(\n", getpid());
        stop();
    }

    if (live(time) != 0) { perror("Visitor: failed to live in the hotel"); return 1; }
    if (leave_hotel() == -1) { perror("Visitor: failed to leave the hotel"); return 1; }
    stop();
    return 0;
}