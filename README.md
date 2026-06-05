# Bootloader
## memory layout
| Address | What's there? |
| --------- | ------------ |
| 0x600 - 0xA00 | Stage 1 bootloader |
| 0xA00 - 0xC00 | Buffer for VBE info |
| 0xC00 - 0xE00 | Buffer for Mode info |
| 0x1000 - 0x1100 | struct videoInfo |
| 0x1100 - 0x1200 | struct bootInfo |
| 0x7c00 - 0xF800 | stage 2 bootloader |


## bootinfo struct

The bootinfo struct is the information that is passed to the kernel.
```
struct bootinfo {
    uint32_t magic;
    struct drive_info boot_drive;
    struct videoInfo video;
    struct elf_info elf;
};
```

```
struct drive_info {
    uint8_t drive;
    uint32_t kernel_partition_lba;
    enum FS fs;
};
```
```
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
```

```
struct elf_info {
    uint32_t entry;
    uint32_t kernel_load_addr;
    uint32_t kernel_size;
};
```

## Required Disk layout

| Sector | What's there |
| ------- | -- |
| 1-2 | Bootsector |
| 2 - 63 | Unallocated (Raw binary) |
| 63 - ??? | Fat 16 with KERNEL.ELF in root dir

## Register state
| Register | What's there |
| ----- | -- |
| rax | boot_info->magic |
| rbx | pointer to boot_info |
| rcx | current instruction pointer |