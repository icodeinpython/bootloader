#include "bootloader.h"

struct bootinfo* boot_info;





void cmain(struct bootinfo* __boot_info) {
    boot_info = __boot_info;
    while (1);
}