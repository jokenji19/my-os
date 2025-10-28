/**
 * @file kernel_minimal.c
 * @brief MINIMAL KERNEL VERSION for testing core functionality
 * This excludes complex components that cause QEMU instability
 */

#include "idt.h"
#include "gdt.h"

/* VGA text mode constants */
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* VGA color attributes */
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_WHITE 15

/**
 * Clear the VGA screen with the given color.
 */
static void vga_clear(unsigned char color) {
    unsigned short *vga = (unsigned short *) VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = vga_entry(' ', color);
    }
}

/**
 * Create a VGA entry with the given character and color.
 */
static unsigned short vga_entry(unsigned char ch, unsigned char color) {
    return (unsigned short) ch | (unsigned short) color << 8;
}

/**
 * Print a string to the VGA screen at the given position with the given color.
 */
static void vga_print(const char *str, int x, int y, unsigned char color) {
    unsigned short *vga = (unsigned short *) VGA_ADDRESS;
    int offset = y * VGA_WIDTH + x;

    while (*str) {
        if (offset >= VGA_WIDTH * VGA_HEIGHT) break;
        vga[offset] = vga_entry(*str, color);
        offset++;
        str++;
    }
}

/* ISR handler functions */
void isr0_handler() {
    vga_print("MINIMAL: Exception handled OK!", 0, 8, VGA_COLOR_WHITE);
}

void isr1_handler() {
    vga_print("MINIMAL: Debug OK!", 0, 9, VGA_COLOR_WHITE);
}

void kernel_minimal(void) {
    /* Initialize Global Descriptor Table */
    init_gdt();

    /* Initialize interrupt descriptor table */
    init_idt();

    /* Clear the screen */
    vga_clear(VGA_COLOR_BLACK);

    /* Basic test messages*/
    vga_print("MY AI-OS KERNEL - MINIMAL VERSION", 0, 0, VGA_COLOR_WHITE);
    vga_print("Testing basic kernel functionality...", 0, 2, VGA_COLOR_WHITE);

    vga_print("GDT/IDT initialized", 0, 4, VGA_COLOR_WHITE);
    vga_print("VGA text mode working", 0, 5, VGA_COLOR_WHITE);
    vga_print("Memory addressing OK", 0, 6, VGA_COLOR_WHITE);

    /* Test basic strings */
    char test_str[] = "Hello from AI kernel!";
    vga_print(test_str, 0, 10, VGA_COLOR_WHITE);

    vga_print("If you see this message, kernel core WORKS!", 0, 12, VGA_COLOR_WHITE);
    vga_print("MINIMAL TEST: SUCCESS!", 0, 14, VGA_COLOR_WHITE);

    /* Halt the system */
    vga_print("System will halt now - kernel functioned!", 0, 16, VGA_COLOR_WHITE);
    __asm__ __volatile__("cli; hlt");
}

/* Loop forever for halt state */
void minimalist_halt(void) {
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
