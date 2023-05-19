#include "msq.h"

#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define ipc_filepath "src/msq/msq.c"

static int string_hash(const char* string)
{
    int ans = 0;
    for (unsigned int i = 0; i < strlen(string); i++) ans = ((ans * 131) + string[i]) % 15999989;
    return ans;
}

struct MessageQueueBuffer
{
    long mtype;
    char mtext[1024]; // Definitely enough for all messages
};
int create_queue(const char* name)
{
    int msq = msgget(IPC_PRIVATE + 1 + ftok(ipc_filepath, string_hash(name)), 0666 | IPC_CREAT);
    return (msq == -1) ? -1 : msq;
}
int write_queue(int msq, const void* src, unsigned int size)
{
    struct MessageQueueBuffer buffer = { .mtype = 1 };
    memcpy(buffer.mtext, src, size); // Copy the data into the buffer
    return (msgsnd(msq, &buffer, size, 0) == -1) ? -1 : 0;
}
int read_queue(int msq, void* dest, unsigned int size)
{
    struct MessageQueueBuffer buffer;
    if (msgrcv(msq, &buffer, size, 1, 0) != size) return -1;
    memcpy(dest, buffer.mtext, size); // Copy the data to the destination
    return 0;
}
int delete_queue(int msq) { return (msgctl(msq, IPC_RMID, NULL) == -1) ? -1 : 0; }