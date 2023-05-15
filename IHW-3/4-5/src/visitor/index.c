#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../protocol.h"

void stop(__attribute__ ((unused)) int signal) { }

int main(void)
{
    setbuf(stdout, NULL); // Remove the buffering of stdout
    siginterrupt(SIGINT, 1);
    signal(SIGINT, stop);

    int client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_address = { .sin_family = AF_INET, .sin_port = htons(8000), .sin_addr = { .s_addr = inet_addr("127.0.0.1") } };
    connect(client, (struct sockaddr *)(&server_address), sizeof(server_address));

    enum Gender gender = FEMALE;
    send(client, &gender, sizeof(gender), 0);

    enum ComeStatus status;
    recv(client, &status, sizeof(status), 0);
    printf("%d", status);

    sleep(5);

    close(client);

    return 0;
}