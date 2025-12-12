#include <system.h>
#include <io.h>

#define EI_NIDENT 16
#define PT_LOAD   1

extern uint32_t partition_lba;
uint32_t mmap_count;

// ---------------- ELF HEADER STRUCTS ----------------

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;     // Entry point
    uint32_t e_phoff;     // Program header table offset
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;     // Number of program headers
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;   // Segment type
    uint32_t p_offset; // Offset in file
    uint32_t p_vaddr;  // Virtual address in memory
    uint32_t p_paddr;
    uint32_t p_filesz; // Bytes in file
    uint32_t p_memsz;  // Bytes in memory (zero padded)
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

// ---------------- ELF LOADER ----------------


uint32_t load_elf32_from_buffer(const uint8_t *elf, uint32_t elf_size) {
    uint32_t begin = 0xFFFFFFFF;
    if (elf_size < sizeof(Elf32_Ehdr)) {
        return -1; // Too small to be ELF
    }

    const Elf32_Ehdr *hdr = (const Elf32_Ehdr *)elf;

    // Check ELF magic
    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L'  || hdr->e_ident[3] != 'F') {
        return -2; // Invalid ELF
    }

    // Only support ELF32 executable
    if (hdr->e_type != 2) { // ET_EXEC
        return -3;
    }

    // Load the program headers
    const Elf32_Phdr *ph = (const Elf32_Phdr *)(elf + hdr->e_phoff);

    for (int i = 0; i < hdr->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD)
            continue;

        uint8_t *dest = (uint8_t *)ph[i].p_vaddr;
        const uint8_t *src = elf + ph[i].p_offset;
        if ((uint32_t)dest < begin) {
            begin = (uint32_t)dest;
        }

        // Copy from file to proper memory addr
        memcpy(dest, src, ph[i].p_filesz);

        printf("Loaded %d bytes from %x to %x\n", ph[i].p_filesz, src, dest);

        // Zero out BSS
        memset(dest + ph[i].p_filesz, 0, ph[i].p_memsz - ph[i].p_filesz);
    }

    printf("Entry point: %x\n", hdr->e_entry);

    bochs_breakpoint();

    boot_info->magic = MAGIC;

    // video
    memcpy(&boot_info->video, video_info, sizeof(struct videoInfo));
    // elf
    boot_info->elf.kernel_load_addr = begin;
    boot_info->elf.kernel_size = elf_size;
    boot_info->elf.entry = hdr->e_entry;
    // drive
    boot_info->boot_drive.drive = fat.drive;
    boot_info->boot_drive.kernel_partition_lba = partition_lba;
    switch (fat.fat_type) {
        case 12:
            boot_info->boot_drive.fs = FS_FAT12;
            break;
        case 16:
            boot_info->boot_drive.fs = FS_FAT16;
            break;
        case 32:
            boot_info->boot_drive.fs = FS_FAT32;
            break;
    }
    // mmap
    boot_info->mmap.mmap = (e820_entry_t*)0x2000;
    boot_info->mmap.mmap_count = mmap_count;

    // Jump to entry point
    entry_func_t entry = (entry_func_t)(hdr->e_entry);

    kernel_jmp(entry);

    return 0;
}