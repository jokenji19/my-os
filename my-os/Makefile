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

# Default target
.PHONY: all clean
all: $(ISO_FILE)

# Clean build files
clean:
	rm -rf build

# Create build directory
build:
	mkdir -p build

# Assembly compilation
build/boot.o: src/boot.s
	mkdir -p build
	$(ASM) -o $@ $<

# Object compilation
build/kernel.o: src/kernel.c src/*.h
	mkdir -p build
	$(GCC) $(GCCFLAGS) -c $< -o $@

build/fat32.o: src/fat32.c src/fat32.h
	mkdir -p build
	$(GCC) $(GCCFLAGS) -c $< -o $@

build/ai_loader.o: src/ai_loader.c src/ai_loader.h
	mkdir -p build
	$(GCC) $(GCCFLAGS) -c $< -o $@

# Additional object files compilation (auto-generated for source files)
define compile-obj
build/$1.o: src/$1.c src/$1.h
	mkdir -p build
	$(GCC) $(GCCFLAGS) -c $$< -o $$@
endef

$(eval $(call compile-obj,kernel))
$(eval $(call compile-obj,ai_runtime))
$(eval $(call compile-obj,sensors))
$(eval $(call compile-obj,memory))
$(eval $(call compile-obj,framebuffer))
$(eval $(call compile-obj,gdt))
$(eval $(call compile-obj,idt))
$(eval $(call compile-obj,pic))
$(eval $(call compile-obj,timer))
$(eval $(call compile-obj,scheduler))
$(eval $(call compile-obj,menu))

# Kernel binary linking (complete version for testing)
$(KERNEL_BIN): build/boot.o build/kernel.o build/ai_runtime.o build/sensors.o build/memory.o build/framebuffer.o build/gdt.o build/idt.o build/pic.o build/timer.o build/scheduler.o build/menu.o
	$(LD) $(LDFLAGS) $^ -o $@

# ISO directory creation
$(ISO_DIR)/boot/kernel.bin: $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin

# GRUB configuration
$(ISO_DIR)/boot/grub/grub.cfg: grub/grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp grub/grub.cfg $(ISO_DIR)/boot/grub/

# ISO image creation
$(ISO_FILE): $(ISO_DIR)/boot/grub/grub.cfg $(ISO_DIR)/boot/kernel.bin
	$(GRUB_MKRESCUE) -o $(ISO_FILE) $(ISO_DIR)
