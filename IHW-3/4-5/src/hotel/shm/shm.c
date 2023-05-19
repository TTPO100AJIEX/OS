#include "shm.h"

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define ipc_filepath "src/hotel/sem/sem.c"
static int string_hash(const char* string)
{
    int ans = 0;
    for (unsigned int i = 0; i < strlen(string); i++) ans = ((ans * 131) + string[i]) % 15999989;
    return ans;
}

struct Memory create_memory(const char* name, size_t size)
{
    // Create the memory instance
    struct Memory mem = { .owner = getpid(), .mem = NULL, .id = shmget(IPC_PRIVATE + 1 + ftok(ipc_filepath, string_hash(name)), size, 0666 | IPC_CREAT) };
    if (mem.id == -1) return mem;
    // Load the memory
    mem.mem = shmat(mem.id, NULL, 0);
    if (!mem.mem) delete_memory(&mem);
    return mem;
}
int delete_memory(struct Memory* mem)
{
    int err = 0; // Save errno to make this function work correctly with perror
    // Close the memory
    if (shmdt(mem->mem) == -1) err = errno;
    // Delete the memory in the parent process
    if (mem->owner == getpid() && shmctl(mem->id, IPC_RMID, NULL) == -1) err = errno;
    // Restore errno and return
    if (err != 0) errno = err;
    return (err == 0) ? 0 : -1;
}