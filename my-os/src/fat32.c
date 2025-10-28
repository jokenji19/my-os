/**
 * @file fat32.c
 * @brief FAT32 File System Driver Implementation
 */

#include "fat32.h"
#include "kernel.h"
#include "memory.h"

/* Forward declarations to avoid conflicts */
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void memcpy(void *dest, const void *src, uint32_t n);

/* Global file system instance */
static fat32_fs_t *fs = 0;

/* IDE controller ports (for sector access) */
#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_SECTOR_CT  0x1F2
#define ATA_LBA_LO     0x1F3
#define ATA_LBA_MI     0x1F4
#define ATA_LBA_HI     0x1F5
#define ATA_DEVICE     0x1F6
#define ATA_COMMAND    0x1F7
#define ATA_STATUS     0x1F7

#define ATA_CMD_READ   0x20
#define ATA_CMD_WRITE  0x30
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_MASTER     0xE0
#define ATA_SLAVE      0xF0

/* Allocate static FS structure */
static uint8_t fs_data[sizeof(fat32_fs_t)];
static fat32_fs_t *global_fs;

/* Initialize FAT32 file system */
int fat32_init(void) {
    global_fs = (fat32_fs_t *)fs_data;

    /* Clear FS structure */
    memset(global_fs, 0, sizeof(fat32_fs_t));

    vga_print("FAT32 driver initialized", 0, 44, VGA_COLOR_LIGHT_CYAN);

    return 0;
}

/* Mount the FAT32 file system */
int fat32_mount(void) {
    uint8_t buffer[FAT32_SECTOR_SIZE];
    int result;

    if (!global_fs) {
        return -1;
    }

    /* Read boot sector (sector 0) */
    result = fat32_read_sector(0, buffer);
    if (result != 0) {
        return -1;
    }

    /* Copy boot sector to FS structure */
    memcpy(global_fs->boot_sector, buffer, FAT32_SECTOR_SIZE);

    /* Parse boot sector structure */
    fat32_boot_sector_t *bs = (fat32_boot_sector_t *)global_fs->boot_sector;
    global_fs->bs = bs;

    /* Verify it's FAT32 */
    if (bs->fs_type[0] != 'F' || bs->fs_type[1] != 'A' || bs->fs_type[2] != 'T' || bs->fs_type[3] != '3' ||
        bs->fs_type[4] != '2') {
        return -2; /* Not FAT32 */
    }

    /* Calculate important values */
    uint32_t fat_size = (bs->fat_size_16 == 0) ? bs->fat_size_32 : bs->fat_size_16;
    uint32_t total_sectors = (bs->total_sectors_16 == 0) ? bs->total_sectors_32 : bs->total_sectors_16;
    uint32_t data_sectors = total_sectors - (bs->reserved_sector_count + (bs->number_of_fats * fat_size) + global_fs->root_directory_sectors);

    global_fs->total_sectors = total_sectors;
    global_fs->data_sectors = data_sectors;
    global_fs->count_of_clusters = data_sectors / bs->sectors_per_cluster;

    /* Calculate starting sectors */
    global_fs->fat_start_sector = bs->reserved_sector_count;
    global_fs->data_start_sector = bs->reserved_sector_count + (bs->number_of_fats * fat_size);

    global_fs->mounted = 1;
    fs = global_fs;

    vga_print("FAT32 filesystem mounted successfully", 0, 45, VGA_COLOR_LIGHT_CYAN);

    return 0;
}

/* Get next cluster from FAT table */
int fat32_get_next_cluster(fat32_fs_t *fs, uint32_t current_cluster) {
    if (!fs || !fs->mounted) {
        return -1;
    }

    /* Calculate which FAT sector contains this cluster */
    uint32_t fat_sector = fs->fat_start_sector + ((current_cluster * 4) / FAT32_SECTOR_SIZE);
    uint32_t fat_offset = (current_cluster * 4) % FAT32_SECTOR_SIZE;

    uint8_t sector_data[FAT32_SECTOR_SIZE];
    int result = fat32_read_sector(fat_sector, sector_data);

    if (result != 0) {
        return -1;
    }

    /* Get the FAT entry (32-bit) */
    uint32_t *fat_entry = (uint32_t *)&sector_data[fat_offset];
    uint32_t next_cluster = *fat_entry & 0x0FFFFFFF; /* Mask out top 4 bits */

    return next_cluster;
}

/* Read a sector from disk */
int fat32_read_sector(uint32_t sector, uint8_t *buffer) {
    /* Wait for drive to be ready */
    while ((inb(ATA_STATUS) & 0x80) == 0x80) {}

    /* Prepare to read */
    outb(ATA_SECTOR_CT, 1);
    outb(ATA_LBA_LO, sector & 0xFF);
    outb(ATA_LBA_MI, (sector >> 8) & 0xFF);
    outb(ATA_LBA_HI, (sector >> 16) & 0xFF);
    outb(ATA_DEVICE, ATA_MASTER | ((sector >> 24) & 0x0F));
    outb(ATA_COMMAND, ATA_CMD_READ);

    /* Wait for read completion */
    while ((inb(ATA_STATUS) & 0x80) == 0x80) {}

    /* Check for errors */
    if (inb(ATA_STATUS) & 0x01) {
        return -1; /* Error */
    }

    /* Read data */
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_DATA);
        buffer[2 * i] = data & 0xFF;
        buffer[2 * i + 1] = (data >> 8) & 0xFF;
    }

    return 0;
}

/* Normalize filename from 8.3 format */
char *fat32_normalize_name(const fat32_dir_entry_t *entry, char *buffer, uint32_t size) {
    uint32_t i = 0;

    /* Copy name part (8 chars) */
    for (int j = 0; j < 8; j++) {
        if (entry->name[j] == ' ') break;
        if (i < size - 1) {
            buffer[i++] = entry->name[j];
        }
    }

    /* Add extension if present */
    uint32_t ext_start = 8;
    uint32_t has_ext = 0;

    for (int j = 8; j < 11; j++) {
        if (entry->name[j] != ' ') {
            has_ext = 1;
            break;
        }
    }

    if (has_ext) {
        if (i < size - 1) {
            buffer[i++] = '.';
        }

        for (int j = 8; j < 11; j++) {
            if (entry->name[j] == ' ') break;
            if (i < size - 1) {
                buffer[i++] = entry->name[j];
            }
        }
    }

    buffer[i] = '\0';
    return buffer;
}

/* Open file for reading */
int fat32_open_file(const char *filename, fat32_file_t *file) {
    if (!fs || !fs->mounted) {
        return -1;
    }

    /* For now, implement simple linear search in root directory */
    uint32_t root_cluster = fs->bs->root_cluster;

    /* Read root directory cluster */
    uint32_t current_cluster = root_cluster;
    uint8_t cluster_data[fs->bs->sectors_per_cluster * FAT32_SECTOR_SIZE];

    /* Simple file search in root (for demo) */
    /* This is a simplified implementation - real FAT32 would handle subdirs */

    return -1; /* File not found or error */
}

/* Basic implementations for now */
int fat32_write_sector(uint32_t sector, const uint8_t *buffer) {
    return -1; /* Not implemented yet */
}

int fat32_close_file(fat32_file_t *file) {
    file->valid = 0;
    return 0;
}

int fat32_read_file(fat32_file_t *file, uint8_t *buffer, uint32_t offset, uint32_t size) {
    return -1; /* Not implemented yet */
}

int fat32_opendir(const char *path, fat32_dir_t *dir) {
    return -1; /* Not implemented yet */
}

int fat32_readdir(fat32_dir_t *dir, fat32_dir_entry_t *entry, char *name, uint32_t name_size) {
    return -1; /* Not implemented yet */
}

int fat32_closedir(fat32_dir_t *dir) {
    return 0;
}

/* In/out functions for kernel */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void memcpy(void *dest, const void *src, uint32_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
}

static inline void memset(void *dest, int val, uint32_t n) {
    unsigned char *ptr = dest;
    while (n--) {
        *ptr++ = val;
    }
}
