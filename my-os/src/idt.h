/**
 * @file idt.h
 * @brief Interrupt Descriptor Table (IDT) management for My OS
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Segment selectors */
#define KERNEL_CS 0x08

/* IDT entry structure */
typedef struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

/* IDT pointer structure */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

/* Function prototypes */
void init_idt();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
extern void isr0();
extern void isr1();
extern void isr32();

#endif
