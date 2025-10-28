/**
 * @file timer.c
 * @brief Implementation of PIT timer
 */

#include "timer.h"
#include "idt.h"
#include "pic.h"
#include "kernel.h"
#include "scheduler.h"

/* Ports for I/O operations */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Global tick counter */
static uint32_t tick_count = 0;

/* Initialize the PIT to generate interrupts at specified frequency */
void pit_init(uint32_t frequency) {
    uint32_t divisor = PIT_FREQUENCY / frequency;

    /* Send the command byte */
    outb(PIT_CMD_REG, PIT_MODE_SQUARE_WAVE | PIT_CMD_CHANNEL0);

    /* Divisor has to be sent byte-wise */
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    outb(PIT_CHANNEL0, low);
    outb(PIT_CHANNEL0, high);
}

/* Timer interrupt handler - called on each timer tick */
void timer_handler() {
    tick_count++;

    /* For stability, only log ticks without complex task switching */
    /* Task switching disabled to prevent interrupt race conditions */

    /* Prevent infinite demo - exit after reasonable period */
    if (tick_count >= 2000) { /* ~20 seconds at 100Hz for quick demo */
        vga_print("Demo AI completata! Spegnimento sicuro...", 0, 45, VGA_COLOR_RED);
        __asm__ __volatile__("cli; hlt");
    }

    /* Send EOI to PIC */
    pic_send_eoi(0);
}

/* Get current tick count */
uint32_t get_tick_count() {
    return tick_count;
}
