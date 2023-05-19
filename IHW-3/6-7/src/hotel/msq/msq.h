#pragma once

// Message queue utilities
int create_queue(const char* name);
int write_queue(int msq, const void* src, unsigned int size);
int read_queue(int msq, void* dest, unsigned int size);
int delete_queue(int msq);