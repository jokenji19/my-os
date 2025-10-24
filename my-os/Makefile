# Makefile for my-os

# Commands
CROSS_PREFIX = i686-elf-
ASM = $(CROSS_PREFIX)as
GCC = $(CROSS_PREFIX)gcc
LD = $(CROSS_PREFIX)ld
OBJCOPY = $(CROSS_PREFIX)objcopy
GRUB_MKRESCUE = $(CROSS_PREFIX)grub-mkrescue

# Flags
ASMFLAGS = 
GCCFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -T src/linker.ld

# Files
BOOT_OBJ = build/boot.o
KERNEL_OBJ = build/kernel.o
OBJS = $(BOOT_OBJ) $(KERNEL_OBJ)
KERNEL_BIN = build/kernel.bin
ISO_DIR = build/isodir
ISO_FILE = build/my-os.iso

.PHONY: all clean run

all: $(ISO_FILE)

run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE)

clean:
	rm -rf build

$(ISO_FILE): $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/
	$(GRUB_MKRESCUE) -o $(ISO_FILE) $(ISO_DIR)

$(KERNEL_BIN): $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_BIN) $(OBJS)

$(BOOT_OBJ): src/boot.s
	mkdir -p build
	$(GCC) -c $< -o $@

$(KERNEL_OBJ): src/kernel.c
	mkdir -p build
	$(GCC) $(GCCFLAGS) -c $< -o $@
