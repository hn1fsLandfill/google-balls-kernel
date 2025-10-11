#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "graphics/graphics.h"

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

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

uint64_t memory_space = 0;

void *malloc(size_t size) {
    // We can get away with this :3
    void *ptr = (void *)memory_space;
    memset(ptr,0,size);
    memory_space += size+256;
    // quick alignment hack
    memory_space -= memory_space%256;
    return ptr;
}


void balls(uint64_t w, uint64_t h, void *framebuffer);

void ps2_init();
void ps2_poll();
void enable_sse();

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // enable them fancy sse but why the fuck do i have to do this anyway
    enable_sse();
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    if (memmap_request.response == NULL) hcf();

    uint64_t entries = memmap_request.response->entry_count;
    uint64_t base = 0;
    uint64_t length = 0;

    for(uint64_t i = 0; i<entries; i++) {
        if(memmap_request.response->entries[i]->type != LIMINE_MEMMAP_USABLE) continue;
        if(memmap_request.response->entries[i]->length < length) continue;

        base = memmap_request.response->entries[i]->base;
        length = memmap_request.response->entries[i]->length;
    }

    memory_space = hhdm_request.response->offset + base;

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    graphics_init(framebuffer->width, framebuffer->height, framebuffer->address);

    if(memory_space == 0x0) {
        // wattesigma
        hcf();
    }

    ps2_init();

    balls(framebuffer->width, framebuffer->height, framebuffer->address);

    // We're done, just hang...
    hcf();
}
