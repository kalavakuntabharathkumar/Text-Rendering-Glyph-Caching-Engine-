#include "framebuffer.h"

#include <stdio.h>
#include <string.h>

int framebuffer_init(FrameBuffer *framebuffer, size_t width, size_t height,
                     MemoryPool *pool)
{
    if (framebuffer == NULL || pool == NULL || width == 0 || height == 0) {
        return 0;
    }
    framebuffer->width = width;
    framebuffer->height = height;
    framebuffer->pool = pool;
    framebuffer->pixels = (unsigned char *)pool_alloc(pool, width * height, 1);
    if (framebuffer->pixels == NULL) {
        framebuffer->width = 0;
        framebuffer->height = 0;
        return 0;
    }
    framebuffer_clear(framebuffer, 0);
    return 1;
}

void framebuffer_clear(FrameBuffer *framebuffer, unsigned char value)
{
    if (framebuffer != NULL && framebuffer->pixels != NULL) {
        memset(framebuffer->pixels, value,
               framebuffer->width * framebuffer->height);
    }
}

void framebuffer_destroy(FrameBuffer *framebuffer)
{
    if (framebuffer != NULL) {
        framebuffer->pixels = NULL;
        framebuffer->width = 0;
        framebuffer->height = 0;
    }
}

int framebuffer_set(FrameBuffer *framebuffer, int x, int y, unsigned char value)
{
    if (framebuffer == NULL || framebuffer->pixels == NULL || x < 0 || y < 0 ||
        (size_t)x >= framebuffer->width || (size_t)y >= framebuffer->height) {
        return 0;
    }
    framebuffer->pixels[(size_t)y * framebuffer->width + (size_t)x] = value;
    return 1;
}

unsigned char framebuffer_get(const FrameBuffer *framebuffer, int x, int y)
{
    if (framebuffer == NULL || framebuffer->pixels == NULL || x < 0 || y < 0 ||
        (size_t)x >= framebuffer->width || (size_t)y >= framebuffer->height) {
        return 0;
    }
    return framebuffer->pixels[(size_t)y * framebuffer->width + (size_t)x];
}

void framebuffer_print_ascii(const FrameBuffer *framebuffer)
{
    size_t x;
    size_t y;
    for (y = 0; y < framebuffer->height; y++) {
        for (x = 0; x < framebuffer->width; x++) {
            putchar(framebuffer->pixels[y * framebuffer->width + x] ? '#' : ' ');
        }
        putchar('\n');
    }
}

int framebuffer_write_ppm(const FrameBuffer *framebuffer, const char *path)
{
    FILE *file;
    size_t x;
    size_t y;
    if (framebuffer == NULL || framebuffer->pixels == NULL || path == NULL) {
        return 0;
    }
    file = fopen(path, "wb");
    if (file == NULL) {
        return 0;
    }
    fprintf(file, "P5\n%lu %lu\n255\n", (unsigned long)framebuffer->width,
            (unsigned long)framebuffer->height);
    for (y = 0; y < framebuffer->height; y++) {
        for (x = 0; x < framebuffer->width; x++) {
            unsigned char value = framebuffer->pixels[y * framebuffer->width + x];
            fputc(value ? 255 : 0, file);
        }
    }
    fclose(file);
    return 1;
}