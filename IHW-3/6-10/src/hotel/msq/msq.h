#pragma once

#include <sys/types.h>
struct MessageQueue
{
    pid_t owner;
    int id;
};

// Message queue utilities
struct MessageQueue create_message_queue(const char* name);
int delete_message_queue(struct MessageQueue* msq);

int write_message_queue(struct MessageQueue* msq, const void* src, unsigned int size);
int read_message_queue(struct MessageQueue* msq, void* dest, unsigned int size);

int message_queue_size(struct MessageQueue* msq);