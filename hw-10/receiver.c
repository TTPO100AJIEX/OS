#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

const struct timespec halfSecond = { 0, 5e8 };

int answer = 0;
pid_t senderPid;

void receiveBit(int signal)
{
    if (signal == SIGUSR1)
    {
        // Received 0
        printf("0"); // Some nice logging :)
        answer <<= 1; // Move the number one bit left. The next bit is set to zero.
    }
    if (signal == SIGUSR2)
    {
        // Received 1
        printf("1"); // Some nice logging :)
        answer = (answer << 1) + 1; // Move the number one bit left and set the next bit to one
    }
    nanosleep(&halfSecond, NULL); // Some nice timeout :)
    kill(senderPid, signal); // Send the callback signal the the sender
}

void stop(int signal)
{
    printf(" = %d", answer); // Print the result
    exit(0); // Stop
}

int main(void)
{
    setbuf(stdout, NULL); // Remove buffering of stdout
    
    printf("I am a receiver. My PID is %d\n", (int)(getpid())); // Print the PID of the receiver
    printf("Please, enter the PID of my sender: "); // Ask for the PID of the sender
    scanf("%d", &senderPid); // Read the PID of the sender
    printf("Received the number "); // Some nice logging :)

    // Register signal listeners for receiving the next bit and for the stop signal
    signal(SIGUSR1, receiveBit);
    signal(SIGUSR2, receiveBit);
    signal(SIGINT, stop);

    while (true) { } // Keep working
}