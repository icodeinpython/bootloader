#include "bootloader.h"

struct bootinfo* boot_info;



void cmain(uint32_t magic, struct bootinfo* __boot_info, uint64_t entry) {
    boot_info = __boot_info;
    while (1);
}