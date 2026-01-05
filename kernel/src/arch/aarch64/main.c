#include <stddef.h>
#include <graphics.h>
#include <mem.h>

// note: this won't boot on anything unless you manually port it
// template for now
// maybe use limine instead????

void platform_init() {
	graphics_init(1280, 720, (void *)0x50000000, 1280*4);
	memory_init(0x40000000, (1024*1024)*256);
}

void imcooked() {
	for(;;) asm("wfi");
}


void kmain_kernel();

__attribute__((section(".text.start"))) void kmain() {
	kmain_kernel();
	imcooked();
}
typedef __uint128_t uint128_t;

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

    return dest;
}

uint64_t timer;
void wait() {

}