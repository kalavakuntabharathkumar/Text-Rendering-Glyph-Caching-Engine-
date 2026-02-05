#include "glyph.h"

#include <stddef.h>

/*
 * Each row is a five-bit packed value. The small 5x7 representation keeps
 * the catalog compact while remaining easy to decompress into pixels.
 */
static const uint8_t patterns[][6] = {
    {' ', 0, 0, 0, 0, 0}, {'!', 0x04, 0x04, 0x04, 0x00, 0x04},
    {'?', 0x0E, 0x11, 0x02, 0x04, 0x04},
    {'0', 0x0E, 0x11, 0x13, 0x15, 0x19},
    {'1', 0x04, 0x0C, 0x04, 0x04, 0x0E},
    {'2', 0x0E, 0x11, 0x02, 0x04, 0x1F},
    {'3', 0x1E, 0x01, 0x06, 0x01, 0x1E},
    {'4', 0x02, 0x06, 0x0A, 0x1F, 0x02},
    {'5', 0x1F, 0x10, 0x1E, 0x01, 0x1E},
    {'6', 0x06, 0x08, 0x1E, 0x11, 0x0E},
    {'7', 0x1F, 0x01, 0x02, 0x04, 0x04},
    {'8', 0x0E, 0x11, 0x0E, 0x11, 0x0E},
    {'9', 0x0E, 0x11, 0x0F, 0x01, 0x06},
    {'A', 0x0E, 0x11, 0x1F, 0x11, 0x11},
    {'B', 0x1E, 0x11, 0x1E, 0x11, 0x1E},
    {'C', 0x0E, 0x11, 0x10, 0x11, 0x0E},
    {'D', 0x1C, 0x12, 0x11, 0x12, 0x1C},
    {'E', 0x1F, 0x10, 0x1E, 0x10, 0x1F},
    {'F', 0x1F, 0x10, 0x1E, 0x10, 0x10},
    {'G', 0x0E, 0x10, 0x17, 0x11, 0x0F},
    {'H', 0x11, 0x11, 0x1F, 0x11, 0x11},
    {'I', 0x0E, 0x04, 0x04, 0x04, 0x0E},
    {'J', 0x07, 0x02, 0x02, 0x12, 0x0C},
    {'K', 0x11, 0x12, 0x1C, 0x12, 0x11},
    {'L', 0x10, 0x10, 0x10, 0x10, 0x1F},
    {'M', 0x11, 0x1B, 0x15, 0x11, 0x11},
    {'N', 0x11, 0x19, 0x15, 0x13, 0x11},
    {'O', 0x0E, 0x11, 0x11, 0x11, 0x0E},
    {'P', 0x1E, 0x11, 0x1E, 0x10, 0x10},
    {'Q', 0x0E, 0x11, 0x11, 0x15, 0x0E},
    {'R', 0x1E, 0x11, 0x1E, 0x12, 0x11},
    {'S', 0x0F, 0x10, 0x0E, 0x01, 0x1E},
    {'T', 0x1F, 0x04, 0x04, 0x04, 0x04},
    {'U', 0x11, 0x11, 0x11, 0x11, 0x0E},
    {'V', 0x11, 0x11, 0x11, 0x0A, 0x04},
    {'W', 0x11, 0x11, 0x15, 0x1B, 0x11},
    {'X', 0x11, 0x0A, 0x04, 0x0A, 0x11},
    {'Y', 0x11, 0x0A, 0x04, 0x04, 0x04},
    {'Z', 0x1F, 0x02, 0x04, 0x08, 0x1F},
    {'.', 0, 0, 0, 0, 0x04}, {',', 0, 0, 0, 0x04, 0x08},
    {'-', 0, 0, 0x1F, 0, 0}, {'+', 0x04, 0x04, 0x1F, 0x04, 0x04},
    {'/', 0x01, 0x02, 0x04, 0x08, 0x10}, {':', 0, 0x04, 0, 0x04, 0},
    {'(', 0x02, 0x04, 0x08, 0x04, 0x02},
    {')', 0x08, 0x04, 0x02, 0x04, 0x08}
};

static const uint8_t *pattern_for(unsigned char codepoint)
{
    size_t i;
    static uint8_t fallback[5];
    for (i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
        if (patterns[i][0] == codepoint) {
            return patterns[i] + 1;
        }
    }
    if (codepoint >= 'a' && codepoint <= 'z') {
        return pattern_for((unsigned char)(codepoint - ('a' - 'A')));
    }
    fallback[0] = (uint8_t)(0x1F & (codepoint >> 0));
    fallback[1] = (uint8_t)(0x1F & (codepoint >> 1));
    fallback[2] = (uint8_t)(0x1F & (codepoint >> 2));
    fallback[3] = (uint8_t)(0x1F & (codepoint >> 3));
    fallback[4] = (uint8_t)(0x1F & (codepoint >> 4));
    return fallback;
}

void glyph_catalog_init(GlyphCatalog *catalog)
{
    int codepoint;
    for (codepoint = GLYPH_FIRST; codepoint <= GLYPH_LAST; codepoint++) {
        GlyphMetadata *entry = &catalog->entries[codepoint - GLYPH_FIRST];
        entry->codepoint = (unsigned char)codepoint;
        entry->width = GLYPH_MAX_WIDTH;
        entry->height = GLYPH_MAX_HEIGHT;
        entry->packed_offset = (unsigned short)((codepoint - GLYPH_FIRST) * 5);
    }
}

const GlyphMetadata *glyph_catalog_find_linear(const GlyphCatalog *catalog,
                                               unsigned char codepoint)
{
    size_t i;
    for (i = 0; i < GLYPH_COUNT; i++) {
        if (catalog->entries[i].codepoint == codepoint) {
            return &catalog->entries[i];
        }
    }
    return NULL;
}

const GlyphMetadata *glyph_catalog_find_binary(const GlyphCatalog *catalog,
                                               unsigned char codepoint)
{
    size_t low = 0;
    size_t high = GLYPH_COUNT;
    while (low < high) {
        size_t middle = low + (high - low) / 2;
        unsigned char candidate = catalog->entries[middle].codepoint;
        if (candidate == codepoint) {
            return &catalog->entries[middle];
        }
        if (candidate < codepoint) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }
    return NULL;
}

size_t glyph_compressed_size(const GlyphMetadata *metadata)
{
    return metadata == NULL ? 0 : 5;
}

int glyph_pixel(unsigned char codepoint, int x, int y)
{
    const uint8_t *pattern;
    if (x < 0 || x >= GLYPH_MAX_WIDTH || y < 0 || y >= GLYPH_MAX_HEIGHT) {
        return 0;
    }
    pattern = pattern_for(codepoint);
    return (pattern[y] & (uint8_t)(1u << (GLYPH_MAX_WIDTH - 1 - x))) != 0;
}

int glyph_decompress(unsigned char codepoint, uint8_t *destination,
                     size_t destination_size, size_t *written)
{
    int x;
    int y;
    if (destination == NULL || destination_size < GLYPH_MAX_BITMAP_BYTES ||
        codepoint < GLYPH_FIRST || codepoint > GLYPH_LAST) {
        return 0;
    }
    for (y = 0; y < GLYPH_MAX_HEIGHT; y++) {
        for (x = 0; x < GLYPH_MAX_WIDTH; x++) {
            destination[y * GLYPH_MAX_WIDTH + x] =
                (uint8_t)glyph_pixel(codepoint, x, y);
        }
    }
    if (written != NULL) {
        *written = GLYPH_MAX_BITMAP_BYTES;
    }
    return 1;
}