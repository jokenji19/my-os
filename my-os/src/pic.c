/**
 * @file pic.c
 * @brief Implementation of PIC 8259 interrupt controller
 */

#include "pic.h"

/* I/O delay - for old hardware compatibility */
void io_wait() {
    __asm__ volatile ("jmp 1f\n\t"
                     "1: jmp 1f\n\t"
                     "1:");
}

/* Ports for I/O operations */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Initialize the PICs and remap IRQs */
void pic_init() {
    /* Master PIC vector offset to 0x20 (32 decimal) */
    uint8_t master_vector = 0x20;
    /* Slave PIC vector offset to 0x28 (40 decimal) */
    uint8_t slave_vector = 0x28;

    /* Save masks */
    uint8_t master_mask = inb(PIC_MASTER_DATA);
    uint8_t slave_mask = inb(PIC_SLAVE_DATA);

    /* ICW1: Start initialization */
    outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC_SLAVE_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: Set vector offsets */
    outb(PIC_MASTER_DATA, master_vector);
    io_wait();
    outb(PIC_SLAVE_DATA, slave_vector);
    io_wait();

    /* ICW3: Tell master there is a slave at IRQ2 (00000100) */
    outb(PIC_MASTER_DATA, 4);
    io_wait();
    /* Tell slave its cascade identity (2) */
    outb(PIC_SLAVE_DATA, 2);
    io_wait();

    /* ICW4: Set 8086 mode */
    outb(PIC_MASTER_DATA, ICW4_8086);
    io_wait();
    outb(PIC_SLAVE_DATA, ICW4_8086);
    io_wait();

    /* Restore saved masks */
    outb(PIC_MASTER_DATA, master_mask);
    outb(PIC_SLAVE_DATA, slave_mask);
}

/* Send End-of-Interrupt signal */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC_SLAVE_CMD, PIC_EOI);
    }
    outb(PIC_MASTER_CMD, PIC_EOI);
}

/* Mask an IRQ */
void pic_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

/* Unmask an IRQ */
void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}
