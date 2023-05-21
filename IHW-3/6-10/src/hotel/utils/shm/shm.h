#pragma once

#include <sys/types.h>
struct Memory
{
    pid_t owner; // Owner of the memory
    int id; // ID of the instance
    void* mem; // Pointer to the memory in the address space of the process
};

// Shared memory utilities
struct Memory create_memory(const char* name, size_t size);
int delete_memory(struct Memory* mem);