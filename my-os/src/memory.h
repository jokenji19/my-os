/**
 * @file memory.h
 * @brief Basic kernel memory management (malloc/kfree)
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define HEAP_START    0x100000  /* Start heap at 1MB */
#define HEAP_SIZE     0x100000  /* 1MB heap size */
#define HEAP_END      (HEAP_START + HEAP_SIZE)

/* Memory block header for linked list allocation */
typedef struct mem_block {
    struct mem_block *next;
    uint32_t size;
    uint8_t free;
} mem_block_t;

#define BLOCK_HEADER_SIZE sizeof(mem_block_t)

/* Function prototypes */
void init_memory_manager();
void *kmalloc(uint32_t size);
void kfree(void *ptr);
void dump_heap();

#endif
