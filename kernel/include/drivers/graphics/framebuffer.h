#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include "../../stdlib/types.h"
#include "../../stdlib/err.h"

struct framebuffer {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t virtWidth;
    uint32_t virtHeight;

    uint32_t pixel_order;

    uint32_t *palette;

    volatile unsigned char *buf;
    uint32_t buf_size;
    uint32_t pitch;
};

enum {
    FONT_WIDTH     = 8,
    FONT_HEIGHT    = 8,
    FONT_BPG       = 8,  // Bytes per glyph
    FONT_BPL       = 1,  // Bytes per line
    FONT_NUMGLYPHS = 224
};

extern unsigned char font[FONT_NUMGLYPHS][FONT_BPG];

extern volatile struct framebuffer fb;

uint32_t init_framebuffer(uint32_t width, uint32_t height, uint32_t depth);

void draw_pixel(int x, int y, uint32_t color);
void draw_char(unsigned char ch, int x, int y, uint32_t color);
void draw_string(int x, int y, char *s, uint32_t color);
void draw_rect(int x1, int y1, int x2, int y2, uint32_t color, int fill);
void draw_circle(int x0, int y0, int radius, uint32_t color, int fill);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);

#endif // _FRAMEBUFFER_H_
