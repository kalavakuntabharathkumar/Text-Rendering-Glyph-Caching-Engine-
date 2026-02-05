#ifndef GLYPH_H
#define GLYPH_H

#include <stddef.h>
#include <stdint.h>

#define GLYPH_FIRST 32
#define GLYPH_LAST 126
#define GLYPH_COUNT (GLYPH_LAST - GLYPH_FIRST + 1)
#define GLYPH_MAX_WIDTH 5
#define GLYPH_MAX_HEIGHT 7
#define GLYPH_MAX_BITMAP_BYTES (GLYPH_MAX_WIDTH * GLYPH_MAX_HEIGHT)

typedef struct {
    unsigned char codepoint;
    unsigned char width;
    unsigned char height;
    unsigned short packed_offset;
} GlyphMetadata;

typedef struct {
    GlyphMetadata entries[GLYPH_COUNT];
} GlyphCatalog;

void glyph_catalog_init(GlyphCatalog *catalog);
const GlyphMetadata *glyph_catalog_find_linear(const GlyphCatalog *catalog,
                                               unsigned char codepoint);
const GlyphMetadata *glyph_catalog_find_binary(const GlyphCatalog *catalog,
                                               unsigned char codepoint);
size_t glyph_compressed_size(const GlyphMetadata *metadata);
int glyph_decompress(unsigned char codepoint, uint8_t *destination,
                     size_t destination_size, size_t *written);
int glyph_pixel(unsigned char codepoint, int x, int y);

#endif