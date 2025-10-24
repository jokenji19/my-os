# I'm defining the Multiboot header constants here.
.set ALIGN,    1<<0             # I need to align modules on page boundaries.
.set MEMINFO,  1<<1             # I need to pass memory map to the kernel.
.set FLAGS,    ALIGN | MEMINFO  # This is the combination of my flags.
.set MAGIC,    0x1BADB002       # This is the magic number for Multiboot.
.set CHECKSUM, -(MAGIC + FLAGS) # This is the checksum to validate the header.

# I'm creating the Multiboot header section.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# This is the main code section.
.section .text
.global start
.type start, @function
start:
    # I'm setting up the stack pointer to point to the top of my stack.
    movl $stack_top, %esp
    
    # Now I'll call my C kernel's main function.
    call kernel_main
    
    # If the kernel ever returns, I'll halt the CPU.
    cli
    hlt
.size start, . - start

# I'm defining the BSS section for my stack.
.section .bss
.align 16
stack_bottom:
.skip 16384 # I'm allocating 16 KiB for the stack.
stack_top: