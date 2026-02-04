#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stddef.h>
#include "memory_pool.h"

typedef struct {
    size_t width;
    size_t height;
    unsigned char *pixels;
    MemoryPool *pool;
} FrameBuffer;

int framebuffer_init(FrameBuffer *framebuffer, size_t width, size_t height,
                     MemoryPool *pool);
void framebuffer_clear(FrameBuffer *framebuffer, unsigned char value);
void framebuffer_destroy(FrameBuffer *framebuffer);
int framebuffer_set(FrameBuffer *framebuffer, int x, int y, unsigned char value);
unsigned char framebuffer_get(const FrameBuffer *framebuffer, int x, int y);
void framebuffer_print_ascii(const FrameBuffer *framebuffer);
int framebuffer_write_ppm(const FrameBuffer *framebuffer, const char *path);

#endif