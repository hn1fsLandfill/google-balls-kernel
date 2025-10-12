#include <stdint.h>
void graphics_init(uint64_t w, uint64_t h, void *fb, uint64_t pitch);
void plot(int x, int y, uint32_t c);
void rect(int x, int y, int w, int h, uint32_t c);
void dot(int x, int y, int r, uint32_t c);
void text(int x, int y, char buf[], uint32_t c);
void sprite(int x, int y, int w, int h, char spr[], uint32_t c1, uint32_t c2);
void flip();

// for debugging
void rect_direct(int x, int y, uint32_t c);
void text_direct(int x, int y, char buf[], uint32_t c);