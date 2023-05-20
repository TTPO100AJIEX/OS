#pragma once

#include <sys/types.h>
struct Memory
{
    pid_t owner;
    int id;
    void* mem;
};

// Shared memory utilities
struct Memory create_memory(const char* name, size_t size);
int delete_memory(struct Memory* mem);