#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <stddef.h>

typedef struct {
    unsigned char *buffer;
    size_t capacity;
    size_t offset;
    size_t current_bytes;
    size_t peak_bytes;
    size_t allocation_count;
} MemoryPool;

void pool_init(MemoryPool *pool, void *buffer, size_t capacity);
void *pool_alloc(MemoryPool *pool, size_t size, size_t alignment);
int pool_free(MemoryPool *pool, void *ptr);
void pool_reset(MemoryPool *pool);
size_t pool_used(const MemoryPool *pool);
size_t pool_peak(const MemoryPool *pool);

#endif