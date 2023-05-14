#include "shm.h"

#define _POSIX_C_SOURCE 200809L // For ftruncate and kill to work properly
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

void* create_memory(const char* name, unsigned int size)
{
    int shm_id = shm_open(name, O_CREAT | O_RDWR, 0666); // Create the memory
    if (shm_id == -1) return NULL;
    if (ftruncate(shm_id, size) == -1) return NULL; // Size the memory
    void* ans = mmap(0, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0); // Load the memory
    return (ans == MAP_FAILED) ? NULL : ans;
}
int delete_memory(const char* name) { return (shm_unlink(name) == -1) ? -1 : 0; }