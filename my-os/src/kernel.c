/**
 * @file kernel.c
 * @brief This is the main function for my kernel.
 *        It's the first C function that gets executed.
 */

#include "idt.h"
#include "gdt.h"
#include "pic.h"
#include "timer.h"
#include "scheduler.h"
#include "memory.h"
#include "framebuffer.h"

/* ISR handler prototypes */
void isr0_handler();
void isr1_handler();

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

/**
 * Create a VGA entry with the given character and color.
 */
unsigned short vga_entry(unsigned char ch, unsigned char color) {
    return (unsigned short) ch | (unsigned short) color << 8;
}

/**
 * Clear the VGA screen with the given color.
 */
void vga_clear(unsigned char color) {
    unsigned short *vga = (unsigned short *) VGA_ADDRESS;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = vga_entry(' ', color);
    }
}

/**
 * Print a string to the VGA screen at the given position with the given color.
 */
void vga_print(const char *str, int x, int y, unsigned char color) {
    unsigned short *vga = (unsigned short *) VGA_ADDRESS;
    int offset = y * VGA_WIDTH + x;
    
    while (*str) {
        if (offset >= VGA_WIDTH * VGA_HEIGHT) break;
        vga[offset] = vga_entry(*str, color);
        offset++;
        str++;
    }
}

/**
 * Calculate the length of a string.
 */
int strlen(const char *str) {
    int len = 0;
    while (*str++) len++;
    return len;
}

/**
 * Copy string.
 */
char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

/**
 * Set memory block to a value.
 */
void *memset(void *dest, int val, int n) {
    unsigned char *ptr = dest;
    while (n--) {
        *ptr++ = val;
    }
    return dest;
}

/**
 * Convert integer to string.
 */
char *itoa(int value, char *str, int base) {
    char *rc;
    char *ptr;
    char *low;
    if (base < 2 || base > 36) {
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }
    low = ptr;
    do {
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[value % base];
        value /= base;
    } while (value);
    *ptr-- = '\0';
    while (low < ptr) {
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

/* ISR handler functions */
void isr0_handler() {
    vga_print("ECCEZIONE: Divisione per zero!", 0, 6, VGA_COLOR_RED);
    while(1); // Halt
}

void isr1_handler() {
    vga_print("ECCEZIONE: Debug interrupt!", 0, 7, VGA_COLOR_RED);
    while(1); // Halt
}

/**
 * Print a string starting from the current cursor position.
 */
void vga_println(const char *str, unsigned char color) {
    static int cursor_x = 0;
    static int cursor_y = 0;
    
    vga_print(str, cursor_x, cursor_y, color);
    
    cursor_x += strlen(str);
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = 0;
        }
    }
}

void kernel_main(void) {
    /* Initialize Global Descriptor Table */
    init_gdt();

    /* Initialize interrupt descriptor table */
    init_idt();

    /* Clear the screen with light grey background */
    vga_clear(VGA_COLOR_BLACK);

    /* Print welcome message */
    vga_print("Benvenuto a My OS - Il futuro del computing mobile!", 0, 0, VGA_COLOR_WHITE);
    vga_print("Sistema operativo AI-centric in sviluppo...", 0, 2, VGA_COLOR_LIGHT_BLUE);
    vga_print("Kernel avviato con successo!", 0, 4, VGA_COLOR_GREEN);

    /* Initialize Programmable Interrupt Controller */
    pic_init();

    /* Initialize timer for ~100 Hz (every 10ms) */
    pit_init(100);

    /* Enable interrupts */
    __asm__ __volatile__("sti");

    vga_print("PIC e Timer inizializzati - Interruzioni abilitate!", 0, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print("Il sistema sta ora ricevendo interrupt del timer...", 0, 10, VGA_COLOR_LIGHT_GREEN);

    /* Initialize the task scheduler */
    init_scheduler();
    /* TODO: Enable tasks when linked properly */
    /* create_task(1, task_process_1);
    create_task(2, task_process_2);
    create_task(3, task_process_3); */

    vga_print("Scheduler inizializzato con 3 processi demo!", 0, 12, VGA_COLOR_LIGHT_GREEN);
    vga_print("Il multitasking e' ora attivo - osserva il switching!", 0, 14, VGA_COLOR_LIGHT_GREEN);

    /* Initialize memory manager */
    init_memory_manager();

    /* Initialize framebuffer for graphics */
    init_framebuffer();

    /* Show framebuffer capabilities demo */
    fb_demo();

    vga_print("GUI preliminare pronta - In attesa di driver hardware!", 0, 30, VGA_COLOR_GREEN);

    /*
     * Main kernel loop - system is now running with interrupts
     * TODO: Add AI reasoning, process scheduling, device management
     */
    while (1) { }
}
