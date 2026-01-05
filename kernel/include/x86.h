#include <stdint.h>

// ps2.c
void ps2_init();
unsigned char ps2_poll();
char get_key();

// config:
// nothing
// no translation
// no clock
// keyboard clock
// nothing
// passed post
// mouse interrupt
// keyboard interrupt
#define PS2_CONFIG 0b00100111

// x86.asm
struct x86_64_regs {
    uint64_t rsp;
    uint64_t rbx;
    uint64_t rbp;
    uint64_t rax;
    uint64_t rdx;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rcx;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} __attribute__((packed));

void enable_sse();
struct x86_64_regs get_all_registers();
void lidt(void *addr);
void imcooked();

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

// interrupts.c
void enable_interrupts();


static inline void io_wait(void)
{
    // outb trick kernel panics
    // outb(0x80, 0);
    for(int i = 0; i<64; i++) { asm("nop"); }
}

// interrupt stuff

#define MPIC_CMD 0x20
#define MPIC_DATA 0x21
#define SPIC_CMD 0xA0
#define SPIC_DATA 0xA1
#define ICW1_INIT	0x10
#define ICW1_ICW4	0x01
#define ICW4_8086	0x01
#define PIC_EOI		0x20

#define MPIC_VECTOR 0x20
#define SPIC_VECTOR 0x28

struct idt64 {
   uint16_t low;
   uint16_t selector;
   uint8_t ist;
   uint8_t type;
   uint16_t mid;
   uint32_t high;
   uint32_t zero;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#define intr_start() { \
    __asm__( \
        "push %rsp\n" \
        "push %rbx\n" \
        "push %rbp\n" \
        "push %rax\n" \
        "push %rdx\n" \
        "push %rdi\n" \
        "push %rsi\n" \
        "push %rdx\n" \
        "push %rcx\n" \
        "push %r8\n" \
        "push %r9\n" \
        "push %r10\n" \
        "push %r11\n" \
        "push %r12\n" \
        "push %r13\n" \
        "push %r14\n" \
        "push %r15\n" \
        "cld\n" \
        "push %rbp\n" \
        "mov %rsp, %rbp\n" \
    ); \
}

#define intr_end() { \
    __asm__( \
        "pop %rbp\n" \
        "pop %r15\n" \
        "pop %r14\n" \
        "pop %r13\n" \
        "pop %r12\n" \
        "pop %r11\n" \
        "pop %r10\n" \
        "pop %r9\n" \
        "pop %r8\n" \
        "pop %rcx\n" \
        "pop %rdx\n" \
        "pop %rsi\n" \
        "pop %rdi\n" \
        "pop %rdx\n" \
        "pop %rax\n" \
        "pop %rbp\n" \
        "pop %rbx\n" \
        "pop %rsp\n" \
        "iretq\n" \
    ); \
}