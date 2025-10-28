/**
 * @file kernel.h
 * @brief Common kernel functions and constants
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

/* VGA text mode constants */
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* VGA color attributes */
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15
#define VGA_COLOR_YELLOW 14  /* Brown is actually yellow in some schemes */
#define VGA_COLOR_PURPLE VGA_COLOR_MAGENTA

/* Function prototypes */
void vga_print(const char *str, int x, int y, unsigned char color);
void vga_clear(unsigned char color);
int strlen(const char *str);
void *memset(void *dest, int val, int n);
char *itoa(int value, char *str, int base);

#endif
