#include <x86.h>
#include <mem.h>
#include <stdint.h>
#include <graphics.h>
#include <balls.h>

__attribute__((naked)) void panic() {
    __asm__("cld");
    __asm__("pop %rax");

    text_direct(10, 10, "oops", 0xff0000);
    text_direct(10, 18, "kernel panic - general protection fault", 0xff0000);

    __asm__("cli");
    for(;;) __asm__("hlt");
}

uint64_t timer = 0;

__attribute__((naked)) void interrupt_timer() {
    intr_start();
    timer++;
    outb(MPIC_CMD, PIC_EOI);
    intr_end();
}

void wait(uint32_t ms) {
    uint64_t newtimer = timer+ms;
    while(newtimer > timer) __asm__("hlt");
}

// > interrupt driven ps2 keyboard driver
// > looks inside
// > state machine
char extended = 0;

__attribute__((naked)) void interrupt_kb() {
    intr_start();
    unsigned char key = ps2_poll();
    if(key == 0xE0) extended++;
    // certified ps2 moment
    else if(extended == 1 && key == 0xF0) extended++;
    else if(extended == 1) {
        extended--;
        switch(key) {
            case 0x75:
                new_kb_event(UP, 1);
                break;
            case 0x72:
                new_kb_event(DOWN, 1);
                break;
            case 0x6B:
                new_kb_event(LEFT, 1);
                break;
            case 0x74:
                new_kb_event(RIGHT, 1);
                break;
            default:
                break;
        }
    } else if(extended == 2) {
        extended -= 2;
        switch(key) {
            case 0x75:
                new_kb_event(UP, 0);
                break;
            case 0x72:
                new_kb_event(DOWN, 0);
                break;
            case 0x6B:
                new_kb_event(LEFT, 0);
                break;
            case 0x74:
                new_kb_event(RIGHT, 0);
                break;
            default:
                break;
        }
    }

    outb(MPIC_CMD, PIC_EOI);
    intr_end();
}

__attribute__((naked)) void interrupt_stub_hw() {
    intr_start();
    outb(MPIC_CMD, PIC_EOI);
    intr_end();
}

__attribute__((naked)) void interrupt_stub() {
    intr_start();
    intr_end();
}

// interrupt(testing123);

enum {
    TRAP = 0,
    INTERRUPT
};

struct idt64 *idts;

void install_interrupt(uint8_t v, void *func, uint8_t type) {
    uint64_t intr = (uint64_t)func;

    idts[v].low = intr & 0xffff;
    idts[v].mid = intr >> 16 & 0xffff;
    idts[v].high = intr >> 32;
    idts[v].ist = 0;
    switch(type) {
        case TRAP:
            idts[v].type = 0x8e;
            break;
        case INTERRUPT:
            idts[v].type = 0x8f;
            break;
    }
    idts[v].selector = 0x28;
    idts[v].zero = 0;
}

void enable_pic() {
    outb(MPIC_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(MPIC_DATA, MPIC_VECTOR);
    io_wait();

    outb(SPIC_DATA, SPIC_VECTOR);
    io_wait();

    outb(MPIC_DATA, 4);
    io_wait();

    outb(SPIC_DATA, 2);
    io_wait();

    outb(MPIC_DATA, ICW4_8086);
    io_wait();

    outb(SPIC_DATA, ICW4_8086);
    io_wait();

    // unmask only the keyboard and clock for now
    outb(MPIC_DATA, 0b11111100);
    outb(SPIC_DATA, 0b11111111);
}

void enable_pit() {
    __asm__("cli");
    outb(0x43, 0b00110100); // channel 0 rate generator

    uint16_t g = 3072;
    outb(0x40, g & 0xff);
    outb(0x40, g >> 8);
    __asm__("sti");
}

void enable_interrupts() {
    idts = malloc(sizeof(struct idt64)*256);
    struct idtr *idt = malloc(sizeof(struct idtr));

    idt->limit = sizeof(struct idt64)*256;
    idt->base = (uint64_t)idts;

    lidt(idt);

    install_interrupt(0x3, interrupt_stub, TRAP);
    install_interrupt(0xD, panic, TRAP);

    for(int off = 0; off<8; off++) {
        install_interrupt(MPIC_VECTOR + off, interrupt_stub_hw, INTERRUPT);
    }

    install_interrupt(MPIC_VECTOR, interrupt_timer, INTERRUPT);
    install_interrupt(MPIC_VECTOR+1, interrupt_kb, INTERRUPT);

    // Quick sanity check
    __asm__("int3");

    enable_pic();
    enable_pit();
}