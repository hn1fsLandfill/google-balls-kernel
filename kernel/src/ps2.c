#include <stdint.h>
#include "graphics/graphics.h"

#define DATA 0x60
#define STATUS 0x64
#define CMD 0x64

// what is this gnu assembly abomination

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
    /* There's an outb %al, $imm8 encoding, for compile-time constant port numbers that fit in 8b. (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static void panic(unsigned char c) {
    // wattesigma
    text_direct(50, 50, "ps2 panic", 0xff0000 | c);
    rect_direct(30, 30, 0xff0000 | c);
    for(;;) { __asm__("hlt"); };
}

unsigned char ps2_poll_wait() {
    for(;;) {
        unsigned char stat = inb(STATUS);
        if(stat & 0x1) {
            return inb(DATA);
        }
    }

    return 0;
}

unsigned char ps2_poll() {
    unsigned char stat = inb(STATUS);
    if(stat & 0x1) {
        return inb(DATA);
    }

    return 0x00;
    //return inb(DATA);
}

void ps2_write(unsigned char data) {
    outb(DATA, data);
    //for(;;) {
    //    unsigned char stat = inb(STATUS);
    //    if(!(stat & 0x1)) {
    //        outb(DATA, data);
    //        return;
    //    } else ps2_poll();
    //}
}

void ps2_init() {
    outb(CMD, 0xAD);
    outb(CMD, 0xA7);
    inb(DATA);
    outb(CMD, 0xAA);
    unsigned char resp = ps2_poll_wait();
    if(resp != 0x55) {
        panic(resp);
    }
    outb(CMD, 0x60);
    outb(DATA, 0b11100100);
    
    outb(CMD, 0xAE);
    outb(DATA, 0xFF);

    while(ps2_poll() != 0x0) {};

    // finally tell them keyboard to use scan code set 2
    resp = 0xFE;
    while(resp != 0xFA) {
        ps2_write(0xF0);
        ps2_write(0x02);
        resp = ps2_poll_wait();
    }

    // set numlock and caps lock led
    resp = 0xFE;
    while(resp != 0xFA) {
        ps2_write(0xED);
        ps2_write(0b011);
        resp = ps2_poll_wait();
    }
//
}