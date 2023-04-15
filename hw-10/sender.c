#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

pid_t receiverPid;
int number;
int curBit = 32;

void send()
{
    if (curBit == 0)
    {
        // 32 bits have been sent - send the end signal to the receiver and stop
        kill(receiverPid, SIGINT);
        exit(0);
    }

    if (number & (1 << (--curBit))) // Check the next bit and subtract 1 from curBit
    {
        // Send SIGUSR2 - the bit is 1
        printf("1"); // Some nice logging :)
        kill(receiverPid, SIGUSR2);
    }
    else
    {
        // Send SIGUSR1 - the bit is 0
        printf("0"); // Some nice logging :)
        kill(receiverPid, SIGUSR1);
    }
}

int main(void)
{
    setbuf(stdout, NULL); // Remove buffering of stdout
    
    printf("I am a sender. My PID is %d\n", (int)(getpid())); // Print the PID of the sender
    printf("Please, enter the PID of my receiver: "); // Ask for the PID of the receiver
    scanf("%d", &receiverPid); // Read the PID of the receiver
    printf("Please, enter the number to send: "); // Ask for the number
    scanf("%d", &number); // Read the number
    printf("Sending the number %d = ", number); // Some nice logging :)

    // Callback signal listeners to get back from the receiver
    signal(SIGUSR1, send);
    signal(SIGUSR2, send);

    // Send the first bit
    send();

    while (true) { } // Keep working
}