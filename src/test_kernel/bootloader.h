#ifndef BOOTLOADER_H
#define BOOTLOADER_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define MAGIC 0xB16B00B5


// provided information: 
// EAX: magic
// EBX: pointer to bootinfo

enum FS {
    FS_FAT12,
    FS_FAT16,
    FS_FAT32,
    FS_EXT2,
    FS_EXT3,
    FS_EXT4
};

struct drive_info {
    uint8_t drive;
    uint32_t kernel_partition_lba;
    enum FS fs;
};

struct videoInfo {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t bytes_per_pixel;
    uint32_t pitch;
    uint32_t screen_size;
    uint32_t screen_size_dqwords;
    uint32_t* framebuffer;
};

struct elf_info {
    uint32_t entry;
    uint32_t kernel_load_addr;
    uint32_t kernel_size;
};

struct bootinfo {
    uint32_t magic;
    struct drive_info boot_drive;
    struct videoInfo video;
    struct elf_info elf;
};

#endif