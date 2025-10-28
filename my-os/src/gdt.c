/**
 * @file gdt.c
 * @brief Implementation of Global Descriptor Table
 */

#include "gdt.h"

/* GDT entries array */
gdt_entry_t gdt_entries[3];
gdt_ptr_t gdt_ptr;

/* Set up a GDT entry */
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F);
    gdt_entries[num].granularity |= (gran & 0xF0);
    gdt_entries[num].access = access;
}

/* Initialize the GDT */
void init_gdt() {
    /* Set up the GDT pointer */
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    /* Null descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Code segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Data segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Load the GDT */
    __asm__ __volatile__("lgdt %0" : : "m"(gdt_ptr));

    /* Reload the segment registers */
    __asm__ __volatile__ (
        "movl %0, %%ds\n"
        "movl %0, %%es\n"
        "movl %0, %%fs\n"
        "movl %0, %%gs\n"
        "movl %0, %%ss\n"
        "ljmp $0x08, $reload_cs\n"
        "reload_cs:"
        : : "r"(0x10)
    );
}
