#include "msq.h"

#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

static int string_hash(const char* string)
{
    int ans = 0;
    for (unsigned int i = 0; i < strlen(string); i++) ans = ((ans * 131) + string[i]) % 15999989;
    return ans;
}

struct MessageQueue create_message_queue(const char* name)
{
    key_t key = ftok(".", string_hash(name)); // Get a key
    msgctl(msgget(key, 0), IPC_RMID, NULL); // Delete old memory queue if exists
    // Create a message queue
    return (struct MessageQueue){ .owner = getpid(), .id = msgget(key, 0666 | IPC_CREAT) };
}
int delete_message_queue(struct MessageQueue* msq)
{
    if (msq->owner != getpid()) return 0; // The message queue does not need to be closed in every child process
    return msgctl(msq->id, IPC_RMID, NULL); // Delete the message queue in the parent process
}


struct MessageQueueBuffer
{
    long mtype;
    char mtext[1024]; // Definitely enough for all messages
};
int write_message_queue(struct MessageQueue* msq, const void* src, unsigned int size)
{
    struct MessageQueueBuffer buffer = { .mtype = 1 };
    memcpy(buffer.mtext, src, size); // Copy the data into the buffer
    buffer.mtext[size] = '\0'; // Add a null terminator
    return msgsnd(msq->id, &buffer, 1024, 0); // Send the data
}
int read_message_queue(struct MessageQueue* msq, void* dest, unsigned int size)
{
    struct MessageQueueBuffer buffer;
    if (msgrcv(msq->id, &buffer, 1024, 1, 0) == -1) return -1; // Get the data    
    memcpy(dest, buffer.mtext, size); // Copy the data to the destination
    return 0;
}