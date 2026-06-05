#include <system.h>

void* memcpy(void *dst, const void *src, uint32_t len) {
    uint8_t *d = (uint8_t*)dst;
    const uint8_t *s = (const uint8_t*)src;
    for (uint32_t i=0;i<len;i++) d[i] = s[i];
    return dst;
}

int memcmp(const void *s1, const void *s2, uint32_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    for (uint32_t i = 0; i < n; ++i) {
        if (p1[i] != p2[i]) {
            return (int)(p1[i] - p2[i]);
        }
    }
    return 0;
}

void* memset(void *dst, int c, uint32_t len) {
    return __builtin_memset(dst, c, len);
}