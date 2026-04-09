default coroutine_create 
default coroutine_yield
default coroutine_init


%define COROUTINE_CAPACITY 10
%define COROUTINE_STACK_SIZE 4*1024

coroutine_create:

; save current context and pass the execution to the scheduler
coroutine_yield:

; init the scheduler 
coroutine_init:

; rdi = function
; this function create a stack to this coroutine
coroutine_go:

; run every coroutine until every coroutine finish
coroutine_run:


section .bss
stack_buffer: resb COROUTINE_CAPACITY*COROUTINE_STACK_SIZE
