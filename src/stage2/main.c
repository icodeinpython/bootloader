#include <system.h>
#include <io.h>
#include <defines.h>

struct videoInfo* video_info = (struct videoInfo*)0x1000;

struct bootinfo* boot_info = (struct bootinfo*)0x1100;
uint32_t partition_lba;

__attribute__((noreturn)) void hcf() {
    while (1) {
        asm volatile ("hlt");
    }
}

__attribute__((noreturn)) void cmain(void) {
    initScreen();
    printf("Hello %d world!\n", 42);

    partition_lba = get_partition_lba(0, 0);
    fat_init(0, partition_lba);
    
    fat_list_dir(fat.root_start, fat.root_entries, 1);
    
    FAT_file file;
    if (fat_find_file(KERNEL_NAME, &file)) {
        printf(KERNEL_NAME " not found\n");
        hcf();
    }
        
    fat_read_file(&file, (void*)0x10000, file.size); // load to 64KiB

    
    bochs_breakpoint();
    load_elf64_from_buffer((uint8_t*)0x10000, file.size); // load the kernel from the buffer at 64KiB


    hcf();
}