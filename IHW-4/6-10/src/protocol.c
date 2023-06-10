#include "protocol.h"

#include <stdio.h>
#include <signal.h>

static void send_message(int socket, void* data, int length, struct sockaddr_in address)
{
    if (sendto(socket, data, length, 0, (struct sockaddr *)(&address), sizeof(address)) == length) return;
    perror("Failed to send a message to the socket");
    raise(SIGINT);
}
void send_request(int socket, struct Request request, struct sockaddr_in address) { send_message(socket, &request, sizeof(request), address); }
void send_response(int socket, struct Response response, struct sockaddr_in address) { send_message(socket, &response, sizeof(response), address); }

static void receive(int socket, void* data, int length, struct sockaddr_in* sender)
{
    socklen_t sender_length = sizeof(struct sockaddr_in);
    if (recvfrom(socket, data, length, 0, (struct sockaddr *)(sender), &sender_length) == length) return;
    perror("Failed to receive a message from the socket");
    raise(SIGINT);
}
struct RequestWrapper receive_request(int socket)
{
    static size_t last_id = 0;
    struct RequestWrapper result = { .id = ++last_id };
    receive(socket, &result.request, sizeof(result.request), &result.client);
    return result;
}
struct ResponseWrapper receive_response(int socket, enum ResponseType type)
{
    struct ResponseWrapper result;
    receive(socket, &result.response, sizeof(result.response), &result.server);
    if (result.response.type != type)
    {
        printf("Received an invalid response type (expected: %d, received: %d)\n", type, result.response.type);
        raise(SIGINT);
    }
    return result;
}