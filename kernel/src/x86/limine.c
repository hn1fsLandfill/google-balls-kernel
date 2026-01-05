#include <limine.h>
#include <stddef.h>
#include <stdbool.h>
#include <graphics.h>
#include <x86.h>
#include <mem.h>

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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_cmdline_request cmdline_request = {
    .id = LIMINE_EXECUTABLE_CMDLINE_REQUEST,
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

bool disableInterrupts = false;

void platform_init() {
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

    // Limine devs are devious for this
    memory_init(hhdm_request.response->offset + base, length);

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    graphics_init(framebuffer->width, framebuffer->height, framebuffer->address, framebuffer->pitch);

    
    char *cmdline = cmdline_request.response->cmdline;

    if(cmdline[0] == 'n' && cmdline[1] == 'o' && cmdline[2] == 'i' && cmdline[3] == 'n' && cmdline[4] == 't') {
        disableInterrupts = true;
    }

    if(!disableInterrupts) enable_interrupts();

    ps2_init();
}