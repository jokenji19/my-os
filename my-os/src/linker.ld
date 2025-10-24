/*
 * I'm setting the entry point of my kernel to the 'start' label,
 * which I've defined in boot.s.
 */
ENTRY(start)

/*
 * Here I define the memory layout for my kernel.
 */
SECTIONS {
    /* I'll start loading my kernel at the 1MB address. */
    . = 1M;

    /* 
     * I'll place the multiboot header and all my executable code 
     * in the .text section.
     */
    .text : {
        *(.multiboot)
        *(.text)
    }

    /* I'll put my read-only data in the .rodata section. */
    .rodata : {
        *(.rodata)
    }

    /* My initialized data goes into the .data section. */
    .data : {
        *(.data)
    }

    /* And my uninitialized data (BSS) will be placed here. */
    .bss : {
        *(.bss)
    }
}