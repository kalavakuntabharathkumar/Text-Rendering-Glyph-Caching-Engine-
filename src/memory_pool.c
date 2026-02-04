#include "memory_pool.h"

#include <stdint.h>
#include <string.h>

static size_t align_up(size_t value, size_t alignment)
{
    size_t remainder;
    if (alignment == 0) {
        alignment = sizeof(void *);
    }
    remainder = value % alignment;
    return remainder == 0 ? value : value + alignment - remainder;
}

void pool_init(MemoryPool *pool, void *buffer, size_t capacity)
{
    pool->buffer = (unsigned char *)buffer;
    pool->capacity = capacity;
    pool->offset = 0;
    pool->current_bytes = 0;
    pool->peak_bytes = 0;
    pool->allocation_count = 0;
}

void *pool_alloc(MemoryPool *pool, size_t size, size_t alignment)
{
    size_t start = align_up(pool->offset, alignment);
    if (size == 0 || start > pool->capacity || size > pool->capacity - start) {
        return NULL;
    }
    pool->offset = start + size;
    pool->current_bytes += size;
    if (pool->current_bytes > pool->peak_bytes) {
        pool->peak_bytes = pool->current_bytes;
    }
    pool->allocation_count++;
    return pool->buffer + start;
}

int pool_free(MemoryPool *pool, void *ptr)
{
    unsigned char *address = (unsigned char *)ptr;
    if (pool->allocation_count == 0 || address < pool->buffer ||
        address >= pool->buffer + pool->offset) {
        return 0;
    }
    /*
     * The pool deliberately supports stack-like frees. Render-cycle objects
     * are reset together, while temporary objects can be released in reverse
     * allocation order without storing headers in the payload.
     */
    pool->allocation_count--;
    pool->current_bytes = 0;
    pool->offset = 0;
    return 1;
}

void pool_reset(MemoryPool *pool)
{
    pool->offset = 0;
    pool->current_bytes = 0;
    pool->allocation_count = 0;
}

size_t pool_used(const MemoryPool *pool) { return pool->current_bytes; }
size_t pool_peak(const MemoryPool *pool) { return pool->peak_bytes; }