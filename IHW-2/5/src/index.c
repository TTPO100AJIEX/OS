#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "log/log.h"
#include "utils/utils.h"
#include "hotel/hotel.h"
#include "visitor/visitor.h"

static bool running = true;
static struct State state;
static pid_t hotelPID;
static void stop()
{
    // If the signal has already been received, do nothing
    if (!running) return;
    running = false;

    printf("\nStopping...\n");
    // Kill the hotel, and wait for the hotel to stop
    if (kill(hotelPID, SIGINT) == -1) printf("Controller: failed to stop the hotel\n");
    waitpid(hotelPID, NULL, 0);

    // Clear all the utilities and exit
    if (clear_state(state) == -1)
    {
        printf("Controller: failed to clear the state\n");
        exit(-1);
    }

    exit(0);
}

int main(int argc, char** argv)
{
    setbuf(stdout, NULL); // Remove buffering of stdout
    signal(SIGINT, stop); // Register the signal handler
    if (argc < 2) { printf("Not enough command line arguments specified!\n"); return 1; }
    // Initialize all the needed utilities
    if (init_state(argv[1], &state) == -1)
    {
        printf("Controller: failed to initialize the state\n");
        clear_state(state);
        return 1;
    }


    // Run the hotel
    hotelPID = fork();
    if (hotelPID == -1) { printf("Controller: failed to create the hotel\n"); clear_state(state); return 1; }
    if (hotelPID == 0) return hotel(state);
    log("Controller: started hotel with PID %d\n", hotelPID);


    bool showMessage = true;
    while (running)
    {
        // Ask for the gender
        if (showMessage) printf("Please enter the gender ('m'/'f') of the next visitor: ");
        char gender = getchar();
        showMessage = false;
        if (gender == EOF) { sleep_milliseconds(5000); continue; } // If stdin is redirected to the file, EOF will be read infinitely
        if (gender <= ' ') continue;
        showMessage = true;
        if (gender != 'm' && gender != 'f') { printf("Gender must be 'm' or 'f'\n"); continue; }

        // Ask for the time
        printf("Please enter the time (in seconds between 1 and 15) the visitor needs a room for: ");
        unsigned int time;
        if (scanf("%u", &time) == EOF) break;
        if (time < 1 || time > 15) { printf("Time must be between 1 and 15\n"); continue; }
        
        // Run the visitor
        pid_t visitorPID = fork();
        if (visitorPID == -1) { printf("Controller: failed to create a visitor\n"); continue; }
        if (visitorPID == 0) return visitor(state, gender == 'm' ? MALE : FEMALE, time);
        log("Controller: started %c visitor with PID %d (time: %u)\n", gender, visitorPID, time);
    }
}