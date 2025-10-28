# Makefile for my-os

# Here I define the cross-compilation tools.
CROSS_PREFIX = i686-elf-
ASM = $(CROSS_PREFIX)as
GCC = $(CROSS_PREFIX)gcc
LD = $(CROSS_PREFIX)ld
OBJCOPY = $(CROSS_PREFIX)objcopy
GRUB_MKRESCUE = $(CROSS_PREFIX)grub-mkrescue

# These are the flags I'll use for the compiler and linker.
ASMFLAGS =
GCCFLAGS = -ffreestanding -O2 -Wall -Wextra -I src
LDFLAGS = -T src/linker.ld

# I'm defining the output and intermediate files here.
BOOT_OBJ = build/boot.o
KERNEL_OBJ = build/kernel.o
KERNEL_MINIMAL_OBJ = build/kernel_minimal.o
IDT_OBJ = build/idt.o
GDT_OBJ = build/gdt.o
FAT32_OBJ = build/fat32.o
AI_LOADER_OBJ = build/ai_loader.o
KERNEL_BIN = build/kernel.bin
KERNEL_MINIMAL_BIN = build/kernel_minimal.bin
ISO_DIR = build/isodir
ISO_FILE = build/my-os.iso
