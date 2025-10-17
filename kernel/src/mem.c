#include <stdint.h>
#include <stddef.h>

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

/*typedef __uint128_t uint128_t;

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    if(!n) return dest;

    while(n%16) {
        *(uint128_t *)pdest = *(uint128_t *)psrc;
        n -= 16;
        psrc += 16;
        pdest += 16;
    }
    while(n%8) {
        *(uint64_t *)pdest = *(uint64_t *)psrc;
        n -= 8;
        psrc += 8;
        pdest += 8;
    }
    while(n%4) {
        *(uint32_t *)pdest = *(uint32_t *)psrc;
        n -= 4;
        psrc += 4;
        pdest += 4;
    }
    while(n%2) {
        *(uint16_t *)pdest = *(uint16_t *)psrc;
        n -= 2;
        psrc += 2;
        pdest += 2;
    }
    while(n) {
        *pdest = *psrc;
        n--;
        pdest++;
        psrc++;
    }
 

    //for (size_t i = 0; i < n/16; i++) {
    //    pdest[i] = psrc[i];
    //}

    return dest;
}*/

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

static uint64_t memory_space = 0;

void imcooked();

void memory_init(uint64_t base, uint64_t size) {
    memory_space = base;
    if(memory_space == 0x0) {
        // wattesigma
        imcooked();
    }
}

void *malloc(size_t size) {
    // We can get away with this :3
    void *ptr = (void *)memory_space;
    memset(ptr,0,size);
    memory_space += size+16;
    // quick alignment hack
    memory_space -= memory_space%16;
    return ptr;
}
