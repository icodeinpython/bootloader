#pragma once


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

#define NULL ((void*)0)

#define true 1
#define false 0

// SCREEN layout:
// offset    | length    | item
// 0         | 4         | width
// 4         | 4         | height
// 8         | 4         | bpp
// 12        | 4         | bytes per pixel
// 16        | 4         | bytes per line (pitch)
// 20        | 4         | screen size
// 24        | 4         | screen_size_dqwords for sse
// 28        | 4         | framebuffer

#include <bootloader.h>

extern struct videoInfo* video_info;

void setPixel(uint32_t x, uint32_t y, uint32_t color);
void setColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void setColor(uint32_t color);
void drawChar(uint32_t x, uint32_t y, char c, uint32_t color);
void initScreen();
void __putchar(unsigned char c);
void __puts(const char* str);
int printf(const char* fmt, ...);
void clearScreen();

typedef __builtin_va_list va_list;

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)
#define va_copy(dest, src) __builtin_va_copy(dest, src)

int read_sector(uint8_t drive, uint32_t lba, uint8_t count, void *buffer);
uint32_t get_partition_lba(uint8_t drive, uint8_t part_num);

typedef struct {
    char name[11];
    uint8_t attr;
    uint16_t first_cluster;
    uint32_t size;
} FAT_file;

typedef struct {
    uint8_t drive;
    uint32_t fat_start;
    uint32_t root_start;
    uint32_t data_start;
    uint32_t root_entries;
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved;
    uint8_t fats;
    uint16_t sectors_per_fat;
    uint32_t root_sectors;
    uint16_t fat_type;
} FAT_info;

int fat_init(uint8_t drive, uint32_t partition_lba);
int fat_find_file(const char *name, FAT_file *file);
int fat_read_file(const FAT_file *file, void *buffer, uint32_t bufsize);

void* memcpy(void *dst, const void *src, uint32_t len);
int memcmp(const void *s1, const void *s2, uint32_t n);
void* memset(void *dst, int c, uint32_t len);

void fat_list_dir(uint32_t dir_start, uint32_t entries_count, int is_root);

extern FAT_info fat;

typedef void (*entry_func_t)(void);

uint32_t load_elf64_from_buffer(const uint8_t *buffer, uint32_t size);
extern void kernel_jmp(entry_func_t addr);

extern struct bootinfo* boot_info;
