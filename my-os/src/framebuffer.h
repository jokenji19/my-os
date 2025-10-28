/**
 * @file framebuffer.h
 * @brief Basic framebuffer driver for graphics display
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

#define FB_WIDTH   640
#define FB_HEIGHT  480
#define FB_BPP     32    /* Bits per pixel */

#define FB_ADDR    0xE0000000  /* High memory address for framebuffer */
#define FB_SIZE    (FB_WIDTH * FB_HEIGHT * FB_BPP / 8)

/* VGA color palette */
#define RGB(r, g, b)  ((uint32_t)(r) << 16 | (uint32_t)(g) << 8 | (uint32_t)(b))

/* Standard colors */
#define COLOR_BLACK         RGB(0, 0, 0)
#define COLOR_WHITE         RGB(255, 255, 255)
#define COLOR_RED           RGB(255, 0, 0)
#define COLOR_GREEN         RGB(0, 255, 0)
#define COLOR_BLUE          RGB(0, 0, 255)
#define COLOR_YELLOW        RGB(255, 255, 0)
#define COLOR_MAGENTA       RGB(255, 0, 255)
#define COLOR_CYAN          RGB(0, 255, 255)
#define COLOR_GRAY          RGB(128, 128, 128)
#define COLOR_LIGHT_BLUE    RGB(173, 216, 230)
#define COLOR_DARK_BLUE     RGB(0, 0, 139)
#define COLOR_LIGHT_GREEN   RGB(144, 238, 144)

/* Function prototypes */
void init_framebuffer();
void fb_putpixel(int x, int y, uint32_t color);
void fb_fill_rect(int x1, int y1, int x2, int y2, uint32_t color);
void fb_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void fb_clear_screen(uint32_t color);
void fb_demo();

#endif
