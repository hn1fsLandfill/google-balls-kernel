#include <stdint.h>
#define GRAPHICS_C
#include <graphics.h>
#include <mem.h>
#define OLIVEC_IMPLEMENTATION
#include <olive.h>

static uint32_t *gfb;
static uint32_t *gbfb;
static Olivec_Canvas gc; 
uint64_t gw;
uint64_t gh;
static uint64_t gp;

void graphics_init(uint64_t w, uint64_t h, void *fb, uint64_t pitch) {
    gfb = fb;
    gbfb = malloc(w*h*4);
    gc = olivec_canvas(gbfb, w, h, w);
    gw = w;
    gh = h;
    gp = pitch/4;
}

void plot(int x, int y, uint32_t c) {
    if(x > gw-1) return;
    else if(x < 0) return;
    if(y > gh-1) return;
    else if(y < 0) return;

    gbfb[y*gp+x] = c;
}

static void plot_direct(int x, int y, uint32_t c) {
    if(x > gw-1) return;
    else if(x < 0) return;
    if(y > gh-1) return;
    else if(y < 0) return;

    gfb[y*gp+x] = c;
}

static void sprite_direct(int x, int y, int w, int h, char spr[], uint32_t c1) {
    for(int y2 = 0; y2<h; y2++)
        for(int x2 = 0; x2<w; x2++) {
            switch(spr[y2*w+x2]) {
                case 1:
                    plot_direct(x+x2,y+y2, c1);
                    break;
            }
        }
}


// for debugging
void rect_direct(int x, int y, uint32_t c) {
    for(int y2 = 0; y2<16; y2++)
        for(int x2 = 0; x2<16; x2++) {
            plot_direct(x+x2,y+y2,c);
        }
}
void text_direct(int x, int y, char buf[], uint32_t c) {
    int x1 = 0;
    while(*buf) {
        sprite_direct(x+x1, y, 6, 6,(char *)olivec_default_glyphs[*buf], c);
        x1 += 6;
        buf++;
    }
}

void rect(int x, int y, int w, int h, uint32_t c) {
    for(int y2 = 0; y2<h; y2++)
        for(int x2 = 0; x2<w; x2++) {
            plot(x+x2,y+y2,c);
        }
}

void horiz_line(int x, int y, int w, uint32_t c) {
    for(int x2 = 0; x2<w; x2++) {
        plot(x+x2,y,c);
    }
}

void dot(int x, int y, int r, uint32_t c) {
    olivec_circle(gc, x, y, r, c);
}

void sprite(int x, int y, int w, int h, char spr[], uint32_t c1, uint32_t c2) {
    for(int y2 = 0; y2<h; y2++)
        for(int x2 = 0; x2<w; x2++) {
            switch(spr[y2*w+x2]) {
                case 1:
                    plot(x+x2,y+y2, c1);
                    break;
                case 2:
                    plot(x+x2,y+y2, c2);
                    break;
            }
        }
}

void text(int x, int y, char buf[], uint32_t c) {
    int x1 = 0;
    while(*buf) {
        sprite(x+x1, y, 6, 6,(char *)olivec_default_glyphs[*buf], c, 0);
        x1 += 6;
        buf++;
    }
}

void flip() {
    memcpy(gfb, gbfb, gw*gh*4);
}