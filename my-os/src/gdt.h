/**
 * @file gdt.h
 * @brief Global Descriptor Table (GDT) management
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* GDT entry structure */
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

/* GDT pointer structure */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* Function prototypes */
void init_gdt();

#endif
