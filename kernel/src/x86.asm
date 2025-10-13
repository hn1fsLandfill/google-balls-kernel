global enable_sse
global rdtsc
global lidt

%macro pushar 0
    push rsp
    push rbx
    push rbp
    push rax
    push rdx
    push rdi
    push rsi
    push rdx
    push rcx
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popar 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rdx
    pop rax
    pop rbp
    pop rbx
    pop rsp
%endmacro

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

lidt:
    cli
    lidt [rdi]
    sti
    ret