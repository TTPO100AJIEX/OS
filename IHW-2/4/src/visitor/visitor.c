#include "visitor.h"

#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include "../log/log.h"
#include "../utils/utils.h"

struct State stateSave;
static void stop()
{
    if (sem_close(stateSave.door) == -1) perror("Hotel: failed to close the door semaphore");
    if (sem_close(stateSave.reception_in) == -1) perror("Hotel: failed to close the reception_in semaphore");
    if (sem_close(stateSave.reception_out) == -1) perror("Hotel: failed to close the reception_out semaphore");
    if (stop_logging() == -1) perror("Hotel: failed to stop the logging");
    exit(0);
}

int visitor(struct State state, enum Gender gender, unsigned int time)
{
    stateSave = state;
    signal(SIGINT, stop);

    if (sem_wait(state.door) == -1) perror("Visitor: failed to close the door semaphore");
    log("Visitor %d: came to the hotel to enter\n", getpid());
    sleep_milliseconds(250);
    struct Request request = { COME_REQUEST, .data = { .come_request = { gender, getpid() } } };
    state.shared_memory->type = REQUEST;
    state.shared_memory->data.request = request;
    if (sem_post(state.reception_in) == -1) perror("Visitor: failed to open the reception_in semaphore");

    if (sem_wait(state.reception_out) == -1) perror("Visitor: failed to close the reception_out semaphore");
    struct ComeResponse come_response = state.shared_memory->data.response.data.come_response;
    if (come_response.status == COME_SORRY)
    {
        log("Visitor %d: left the hotel, there were no places for me :(\n", getpid());
    }
    else
    {
        log("Visitor %d: started sleeping\n", getpid());
        sleep_milliseconds(time * 1000);
        log("Visitor %d: stopped sleeping\n", getpid());
        
        if (sem_wait(state.door) == -1) perror("Visitor: failed to close the door semaphore");
        log("Visitor %d: came to the hotel to leave\n", getpid());
        sleep_milliseconds(250);
        struct Request request = { LEAVE_REQUEST, .data = { .leave_request = { getpid() } } };
        state.shared_memory->type = REQUEST;
        state.shared_memory->data.request = request;
        if (sem_post(state.reception_in) == -1) perror("Visitor: failed to open the reception_in semaphore");
        
        if (sem_wait(state.reception_out) == -1) perror("Visitor: failed to close the reception_out semaphore");
        log("Visitor %d: left the hotel, OK\n", getpid());
    }

    if (sem_close(state.door) == -1) perror("Hotel: failed to close the door semaphore");
    if (sem_close(state.reception_in) == -1) perror("Hotel: failed to close the reception_in semaphore");
    if (sem_close(state.reception_out) == -1) perror("Hotel: failed to close the reception_out semaphore");
    if (stop_logging() == -1) perror("Hotel: failed to stop the logging");
    return 0;
}