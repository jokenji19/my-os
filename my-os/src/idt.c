/**
 * @file idt.c
 * @brief Implementation of Interrupt Descriptor Table for My OS
 */

#include "idt.h"

/* Declare an IDT of 256 entries */
idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

/* Function to set an IDT gate */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = selector;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = flags;
}

/* Function to initialize the IDT */
void init_idt() {
    uint32_t i;

    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt_entries;

    /* Clear the IDT - manually instead of memset */
    for (i = 0; i < sizeof(idt_entries); i++) {
        ((uint8_t*)idt_entries)[i] = 0;
    }

    /* Set up the IDT gates */
    idt_set_gate(0, (uint32_t)isr0, KERNEL_CS, 0x8E);   // Divide by zero
    idt_set_gate(1, (uint32_t)isr1, KERNEL_CS, 0x8E);   // Debug
    idt_set_gate(32, (uint32_t)isr32, KERNEL_CS, 0x8E); // Timer (IRQ 0)

    /* Load the IDT */
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
}
