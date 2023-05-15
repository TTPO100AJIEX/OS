#pragma once

// Shared memory utilities
void* create_memory(const char* name, unsigned int size);
int delete_memory(const char* name);