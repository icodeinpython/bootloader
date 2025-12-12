#include <bootloader.h>

struct bootinfo* boot_info;


void clearScreen() {
    uint32_t screen_size = boot_info->video.pitch * boot_info->video.height;
    for (uint32_t i = 0; i < screen_size/4; i++) {
        boot_info->video.framebuffer[i] = 0x00000000;
    }
}


void cmain(struct bootinfo* __boot_info) {
    __asm__ ("xchg %bx, %bx");
    boot_info = __boot_info;
    clearScreen();
    while (1);
}