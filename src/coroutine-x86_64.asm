default rel
global __get_stack_base
global __switch_context

%define COROUTINE_CAPACITY 10
%define COROUTINE_STACK_SIZE 4*1024

section .text

; rdi, rsi = {
;    void *rsp;
;    void *rbp;
;    void *rbx;
;    void *r12;
;    void *r13;
;    void *r14;
;    void *r15;
;}
; rdi = from
; rsi = to
__switch_context:
    mov [rdi], rsp
    mov [rdi + 8], rbp
    mov [rdi + 16], rbx
    mov [rdi + 24], r12
    mov [rdi + 32], r13
    mov [rdi + 40], r14
    mov [rdi + 48], r15

    mov rsp, [rsi + 0]
    mov rbp, [rsi + 8]
    mov rbx, [rsi + 16]
    mov r12, [rsi + 24]
    mov r13, [rsi + 32]
    mov r14, [rsi + 40]
    mov r15, [rsi + 48]

    ret

;      -----------------------
; rsp | coroutine_bootstrap | 8 bytes
;     -----------------------
;     |     return rip      | 8 bytes
;     -----------------------
;     |    stack frame     |
__get_stack_base:
    cmp rdi, COROUTINE_CAPACITY 
    jge .overflow
    mov rax, rdi
    imul rax, COROUTINE_STACK_SIZE 
    lea rdx, [stack_buffer]
    add rax, rdx
    ret
.overflow:
    mov rax, -1
    ret

section .bss
stack_buffer: resb COROUTINE_CAPACITY*COROUTINE_STACK_SIZE
