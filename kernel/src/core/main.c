#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <balls.h>

void platform_init();
void imcooked();

void kmain_kernel(void) {
    platform_init();

    balls();

    // We're done, just hang...
    imcooked();
}
