default __save_context 


%define COROUTINE_CAPACITY 10
%define COROUTINE_STACK_SIZE 4*1024


;typedef struct {
;    void *rsp;
;    void *rbp;
;    void *rbx;
;    void *r12;
;    void *r13;
;    void *r14;
;    void *r15;
;    void *rip;
;} context_s;
; rdi = context_s

__save_context:
    mov [rdi], rsp
    mov [rdi + 8], rbp
    mov [rdi + 16], rbx
    mov [rdi + 24], r12
    mov [rdi + 32], r13
    mov [rdi + 40], r14
    mov [rdi + 48], r15
    mov [rdi + 56], rip
    ret

section .bss
stack_buffer: resb COROUTINE_CAPACITY*COROUTINE_STACK_SIZE
