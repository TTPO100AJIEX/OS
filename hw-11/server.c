#include <sys/socket.h>
#include <arpa/inet.h>

int main(void)
{
    int server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in echoServAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { .s_addr = htonl(INADDR_ANY) }
    };
    bind(server, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
}