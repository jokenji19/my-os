/**
 * @file memory.c
 * @brief Implementation of kernel memory management (kmalloc/kfree)
 */

#include "memory.h"
#include "kernel.h"

/* Global heap start and first block */
static mem_block_t *heap_start = 0;

/* Initialize the memory manager */
void init_memory_manager() {
    /* Set up initial block covering entire heap */
    heap_start = (mem_block_t *)HEAP_START;
    heap_start->size = HEAP_SIZE - BLOCK_HEADER_SIZE;
    heap_start->free = 1;
    heap_start->next = 0;

    vga_print("Memory manager inizializzato - Heap: 1MB disponibile", 0, 16, VGA_COLOR_WHITE);
}

/* Find a free block using first-fit algorithm */
static mem_block_t *find_free_block(uint32_t size) {
    mem_block_t *current = heap_start;

    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }

    return 0; /* No suitable block found */
}

/* Split a block if it's too large */
static void split_block(mem_block_t *block, uint32_t size) {
    if (block->size >= size + BLOCK_HEADER_SIZE + 1) { /* Minimum split size */
        mem_block_t *new_block = (mem_block_t *)((uint32_t)block + BLOCK_HEADER_SIZE + size);
        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

/* Merge adjacent free blocks */
static void merge_blocks() {
    mem_block_t *current = heap_start;

    while (current && current->next) {
        if (current->free && current->next->free) {
            /* Merge current and next block */
            current->size += BLOCK_HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

/* Allocate memory */
void *kmalloc(uint32_t size) {
    if (size == 0) return 0;

    /* Align size to 4 bytes for better performance */
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }

    mem_block_t *block = find_free_block(size);
    if (!block) {
        return 0; /* Out of memory */
    }

    /* Split block if necessary */
    split_block(block, size);

    /* Mark block as used */
    block->free = 0;

    /* Return address after header */
    return (void *)((uint32_t)block + BLOCK_HEADER_SIZE);
}

/* Free memory */
void kfree(void *ptr) {
    if (!ptr) return;

    /* Get block header */
    mem_block_t *block = (mem_block_t *)((uint32_t)ptr - BLOCK_HEADER_SIZE);

    /* Mark block as free */
    block->free = 1;

    /* Merge adjacent free blocks */
    merge_blocks();
}

/* Debug function to dump heap status */
void dump_heap() {
    mem_block_t *current = heap_start;
    int count = 0;

    vga_print("Heap dump:", 0, 18, VGA_COLOR_WHITE);

    while (current && count < 5) { /* Show first 5 blocks */
        char buffer[32];

        itoa(current->size, buffer, 10);

        vga_print("Block ", 0, 19 + count, current->free ? VGA_COLOR_GREEN : VGA_COLOR_RED);
        vga_print(current->free ? "FREE" : "USED", 8, 19 + count, current->free ? VGA_COLOR_GREEN : VGA_COLOR_RED);
        vga_print(" ", 13, 19 + count, VGA_COLOR_WHITE);
        vga_print(buffer, 15, 19 + count, VGA_COLOR_CYAN);
        vga_print(" bytes", 25, 19 + count, VGA_COLOR_WHITE);

        count++;
        current = current->next;
    }

    if (!current) {
        vga_print("(end)", 0, 19 + count, VGA_COLOR_LIGHT_GREY);
    }
}
