global enable_sse
global rdtsc

.text:
enable_sse:
    push rax
    mov rax, cr0
    and ax, 0xFFFB
    or ax, 0x2
    mov cr0, rax
    mov rax, cr4
    or ax, 3 << 9
    mov cr4, rax
    pop rax
    ret

rdtsc:
    push rdx
    rdtsc
    shl rdx, 32
    or rax, rdx
    pop rdx
    ret