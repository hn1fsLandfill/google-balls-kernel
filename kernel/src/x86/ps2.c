#include <stdint.h>
#include <stdbool.h>
#include <graphics.h>
#include <x86.h>

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
}

void ps2_controller_init() {
    unsigned char resp;

    // disable all ps2 stuff 
    outb(CMD, 0xAD);
    outb(CMD, 0xA7);
    
    // make sure our buffer is empty
    inb(DATA);
    
    // test keyboard
    outb(CMD, 0xAA);
    resp = ps2_poll_wait();
    if(resp != 0x55) {
        panic(resp);
    }
    
    // set PS2 config
    outb(CMD, 0x60);
    outb(DATA, PS2_CONFIG);
    
    // re-enable keyboard
    outb(CMD, 0xAE);

    // reset devices
    outb(DATA, 0xFF);

    while(ps2_poll() != 0x0) {};
}

void ps2_kb_init() {
    int resp;

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
        ps2_write(0b110);
        resp = ps2_poll_wait();
    }

    // enable scancodes
    resp = 0xFE;
    while(resp != 0xFA) {
        ps2_write(0xF4);
        resp = ps2_poll_wait();
    }

    ps2_poll();
}

void ps2_init() {
    __asm__("cli");
    ps2_controller_init();
    ps2_kb_init();
    // TODO: Actually get PS2 interrupts working
    outb(CMD, 0x60);
    //// set config to
    //// keyboard interrupt
    //// no interrupt
    //// passed post
    //// nothing
    //// clock
    //// clock
    //// no translation
    //// nothing
    //outb(DATA, 0b00110101);
    __asm__("sti");
}