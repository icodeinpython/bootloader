#include <system.h>

static void print_uint(uint32_t v, uint32_t base, int upper) {
    char buf[16];
    int i = 0;
    if (v == 0) {
        __putchar('0');
        return;
    }

    while (v > 0) {
        uint32_t digit = v % base;
        v /= base;
        if (digit < 10) {
            buf[i++] = '0' + digit;
        } else {
            buf[i++] = (upper ? 'A' : 'a') + (digit - 10);
        }
    }

    while (i--)
        __putchar(buf[i]);
}

static void print_int(int32_t v) {
    if (v < 0) {
        __putchar('-');
        v = -v;
    }
    print_uint((uint32_t)v, 10, 0);
}

int printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            __putchar(*fmt++);
            continue;
        }

        fmt++;
        switch (*fmt) {
        case 's':
            const char* s = va_arg(ap, const char*);
            __puts(s ? s : "(null)");
            break;
        case 'c':
            __putchar((char)va_arg(ap, int));
            break;
        case 'd':
        case 'i':
            print_int(va_arg(ap, int));
            break;
        case 'u':
            print_uint(va_arg(ap, uint32_t), 10, 0);
            break;
        case 'x':
            print_uint(va_arg(ap, uint32_t), 16, 0);
            break;
        case 'X':
            print_uint(va_arg(ap, uint32_t), 16, 1);
            break;
        case 'p':
            __puts("0x");
            print_uint((uint32_t)va_arg(ap, void*), 16, 0);
            break;
        case '%':
            __putchar('%');
            break;
        }
        fmt++;
    }

    va_end(ap);
    return 0;
}