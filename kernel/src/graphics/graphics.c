#include <stdint.h>
#include "graphics.h"
#include "mem.h"
#define OLIVEC_IMPLEMENTATION
#include "olive.h"

static uint32_t *gfb;
static uint32_t *gbfb;
static Olivec_Canvas gc; 
static uint64_t gw;
static uint64_t gh;

void graphics_init(uint64_t w, uint64_t h, void *fb) {
    gfb = fb;
    gbfb = malloc(w*h*4);
    gc = olivec_canvas(gbfb, w, h, w);
    gw = w;
    gh = h;
}

void plot(int x, int y, uint32_t c) {
    if(x > gw-1) return;
    else if(x < 0) return;
    if(y > gh-1) return;
    else if(y < 0) return;

    gbfb[y*gw+x] = c;
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
    //int ox, oy;
    //ox = x-(r/2);
    //oy = y-(r/2);
//
    //for (int y1 = -r; y1 <= r; y1++)
    //    for (int x1 = -r; x1 <= r; x1++)
    //        if ((x1 * x1) + (y1 * y1) <= (r * r))
    //            plot(ox+x1, oy+y1, c);
}

void text(int x, int y, char buf[], uint32_t c) {
    olivec_text(gc, buf, x, y, olivec_default_font, 2, c);
}

void flip() {
    memcpy(gfb, gbfb, gw*gh*4);
}