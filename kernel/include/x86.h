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
// no mouse interrupt
// keyboard interrupt
#define PS2_CONFIG 0b00100101

// x86.asm
void enable_sse();
void lidt(void *addr);
void imcooked();

// interrupts.c
void enable_interrupts();

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

static inline void io_wait(void)
{
    outb(0x80, 0);
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