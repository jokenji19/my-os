/**
 * @file fat32.h
 * @brief FAT32 File System Driver Header
 */

#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

/* FAT32 Data Structures */
#define FAT32_SECTOR_SIZE 512
#define FAT32_MAX_FILENAME 255

/* FAT32 Boot Sector */
typedef struct __attribute__((packed)) {
    uint8_t boot_jump[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t number_of_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_descriptor;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;

    /* FAT32 Extended Boot Record */
    uint32_t fat_size_32;
    uint16_t extended_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
    uint8_t boot_code[420];
    uint16_t boot_sector_signature;
} fat32_boot_sector_t;

/* FAT32 Directory Entry */
typedef struct __attribute__((packed)) {
    uint8_t name[11];
    uint8_t attr;
    uint8_t nt_reserved;
    uint8_t time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t first_cluster_hi;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_lo;
    uint32_t size;
} fat32_dir_entry_t;

/* FAT32 Long File Name Entry */
typedef struct __attribute__((packed)) {
    uint8_t seq_number;
    uint8_t name1[10];
    uint8_t attr;
    uint8_t type;
    uint8_t checksum;
    uint8_t name2[12];
    uint16_t first_cluster_lo;
    uint8_t name3[4];
} fat32_lfn_entry_t;

/* Directory attributes */
#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN    0x02
#define FAT32_ATTR_SYSTEM    0x04
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE   0x20
#define FAT32_ATTR_LFN       0x0F

/* FAT32 File System Information */
typedef struct {
    uint8_t boot_sector[FAT32_SECTOR_SIZE];  /* Raw boot sector data */
    fat32_boot_sector_t *bs;                 /* Parsed boot sector */

    /* Calculated values */
    uint32_t root_directory_sectors;
    uint32_t total_sectors;
    uint32_t data_sectors;
    uint32_t count_of_clusters;

    /* Key locations */
    uint32_t fat_start_sector;
    uint32_t root_start_sector;
    uint32_t data_start_sector;

    /* Status */
    uint8_t mounted;
} fat32_fs_t;

/* File handle for opened files */
typedef struct {
    fat32_fs_t *fs;
    fat32_dir_entry_t dir_entry;
    uint32_t current_cluster;
    uint32_t position;
    uint32_t size;
    uint8_t valid;
} fat32_file_t;

/* Directory iterator */
typedef struct {
    fat32_fs_t *fs;
    uint32_t current_cluster;
    uint32_t current_sector;
    uint32_t current_entry;
    uint8_t valid;
} fat32_dir_t;

/* Function prototypes */
int fat32_init(void);
int fat32_mount(void);
int fat32_get_next_cluster(fat32_fs_t *fs, uint32_t current_cluster);
int fat32_read_sector(uint32_t sector, uint8_t *buffer);
int fat32_write_sector(uint32_t sector, const uint8_t *buffer);

char *fat32_normalize_name(const fat32_dir_entry_t *entry, char *buffer, uint32_t size);
int fat32_open_file(const char *filename, fat32_file_t *file);
int fat32_close_file(fat32_file_t *file);
int fat32_read_file(fat32_file_t *file, uint8_t *buffer, uint32_t offset, uint32_t size);

int fat32_opendir(const char *path, fat32_dir_t *dir);
int fat32_readdir(fat32_dir_t *dir, fat32_dir_entry_t *entry, char *name, uint32_t name_size);
int fat32_closedir(fat32_dir_t *dir);

#endif
