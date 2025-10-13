#include <stdint.h>
#include "graphics/graphics.h"
#include "x86.h"

#define DATA 0x60
#define STATUS 0x64
#define CMD 0x64

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

__attribute__((naked)) void interrupt_kb() {
    intr_start();
    inb(DATA);
    rect_direct(0,0,0xff00ff);
    intr_end();
}

void ps2_init() {
    __asm__("cli");
    outb(CMD, 0xAD);
    outb(CMD, 0xA7);
    ps2_poll();
    outb(CMD, 0xAA);
    unsigned char resp = ps2_poll_wait();
    if(resp != 0x55) {
        panic(resp);
    }
    outb(CMD, 0x60);
    outb(DATA, 0b00100100);
    
    // re enable keyboard and enable irqs
    outb(CMD, 0xAE);

    // reset devices
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
        ps2_write(0b001);
        resp = ps2_poll_wait();
    }

    // enable scancodes
    resp = 0xFE;
    while(resp != 0xFA) {
        ps2_write(0xF6);
        resp = ps2_poll_wait();
    }

    // TODO: Actually get PS2 interrupts working
    outb(CMD, 0x60);
    outb(DATA, 0b11100100);

    ps2_poll();
    __asm__("sti");
//
}
