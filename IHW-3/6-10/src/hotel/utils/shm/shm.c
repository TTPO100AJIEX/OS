#include "shm.h"

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

static int string_hash(const char* string)
{
    int ans = 0;
    for (unsigned int i = 0; i < strlen(string); i++) ans = ((ans * 131) + string[i]) % 15999989;
    return ans;
}

struct Memory create_memory(const char* name, size_t size)
{
    key_t key = ftok(".", string_hash(name)); // Get a key
    shmctl(shmget(key, size, 0), IPC_RMID, NULL); // Delete old memory if it exists
    // Create the memory instance
    struct Memory mem = { .owner = getpid(), .mem = NULL, .id = shmget(key, size, 0666 | IPC_CREAT) };
    if (mem.id == -1) return mem;
    // Load the memory
    mem.mem = shmat(mem.id, NULL, 0);
    if (!mem.mem) delete_memory(&mem);
    return mem;
}
int delete_memory(struct Memory* mem)
{
    int status = 0;
    // Close the memory in every process
    if (shmdt(mem->mem) == -1) status = -1;
    // Delete the memory in the parent process
    if (mem->owner == getpid() && shmctl(mem->id, IPC_RMID, NULL) == -1) status = -1;
    return status;
}