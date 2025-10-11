#include <stdint.h>
void graphics_init(uint64_t w, uint64_t h, void *fb);
void plot(int x, int y, uint32_t c);
void rect(int x, int y, int w, int h, uint32_t c);
void dot(int x, int y, int r, uint32_t c);
void text(int x, int y, char buf[], uint32_t c);