#include <system.h>
#include <vga_font.h>


uint32_t cursor_x = 0, cursor_y = 0;
uint32_t max_x, max_y;

typedef union {
    uint32_t value;
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
} color_t;

color_t g_color;

void setPixel(uint32_t x, uint32_t y, uint32_t color) {
    video_info->framebuffer[y * (video_info->pitch / video_info->bytes_per_pixel) + x] = color;
}

void setColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    g_color.r = r;
    g_color.g = g;
    g_color.b = b;
    g_color.a = a;
}

void setColor(uint32_t color) {
    g_color.value = color;
}

void drawChar(uint32_t x, uint32_t y, char c, uint32_t color) {
    for (int i = 0; i < 8; i++) {
        uint8_t row = font8x8_basic[(uint8_t)c][i];
        for (int j = 0; j < 8; j++) {
            if (row & (1 << j)) {
                setPixel(x+j, y+i, color);
            }
        }
    }
}

void initScreen() {
    max_x = video_info->width / 8;
    max_y = video_info->height / 8;
    cursor_x = 0;
    cursor_y = 0;

    setColorRGBA(0, 0xFF, 0, 0);
}

void __putchar(unsigned char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        cursor_x--;
        for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
                setPixel((cursor_x*8)+j, (cursor_y*8)+j, 0);
            }
        }
    } else {
        drawChar(cursor_x*8, cursor_y*8, c, g_color.value);
        cursor_x++;
        if (cursor_x >= max_x) {
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= max_y) {
                cursor_y = 0;
            }
			for (uint32_t i = 0; i < max_x; i++) {
				drawChar(i * 8, cursor_y * 8, ' ', 0x00000000);
			}
        }
    }
}

void __puts(const char* str) {
    while (*str) {
        __putchar(*str++);
    }
}

void clearScreen() {
    memset(video_info->framebuffer, 0, video_info->pitch * video_info->height);
    cursor_x = 0;
    cursor_y = 0;
}