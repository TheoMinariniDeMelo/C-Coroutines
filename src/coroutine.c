#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#define public __attribute((visibility("default")))
#define private __attribute((visibility("hidden")))

#define COROUTINES_CAPACITY 10
#define MAX_STACK_SIZE (4*1024)

typedef struct {
    void *rsp;
    void *rbp;
    void *rbx;
    void *r12;
    void *r13;
    void *r14;
    void *r15;
} context_s;

typedef enum {
    CO_RUNNING,
    CO_FINISHED,
    CO_WAITING,
    CO_READY
} coroutine_status;

typedef struct {
    context_s ctx;
    void (*func)(void);
    void* stack;
    size_t stack_size;
    coroutine_status status;
} coroutine_s;

typedef struct {
    coroutine_s coroutines[COROUTINES_CAPACITY];
    context_s ctx;
    int current_idx;
    int count;
} scheduler_s;

scheduler_s *scheduler = NULL;

private extern void __switch_context(context_s* from, context_s* to);
private extern void* __get_stack_base(int idx);
public int coroutine_completed(void);

private int choose_next_ready(void){
    if(scheduler->count <= 0) return -1;

    int start = scheduler->current_idx;

    for(int off = 1; off <= scheduler->count; ++off){
        int i = (start + off + scheduler->count) % scheduler->count;
        if(scheduler->coroutines[i].status == CO_READY) return i;
    }
    return -1;
}

private int choose_next_finished(void){
    if(scheduler->count <= 0) return -1;

    int start = scheduler->current_idx;

    for(int off = 1; off <= scheduler->count; ++off){
        int i = (start + off + scheduler->count) % scheduler->count;
        if(scheduler->coroutines[i].status == CO_FINISHED) return i;
    }
    return -1;
}

private void scheduler_run(){
    int next;
    while(!coroutine_completed()){
        next = choose_next_ready();
        if(next < 0) continue;
        scheduler->current_idx = next;
        scheduler->coroutines[next].status = CO_RUNNING;
        __switch_context(&scheduler->ctx, &scheduler->coroutines[next].ctx);
    }
}

private void coroutine_bootstrap(){
    int idx = scheduler->current_idx;
    coroutine_s *co = &scheduler->coroutines[idx];

    co->status = CO_RUNNING;
    co->func();
    co->status = CO_FINISHED;

    __switch_context(&co->ctx, &scheduler->ctx);

    __builtin_unreachable();
}

private void create_coroutine(int i, void(*func)(void)){
    coroutine_s *co = &scheduler->coroutines[i];
    co->func = func;
    co->status = CO_READY;
    co->stack = __get_stack_base(i);
    co->stack_size = MAX_STACK_SIZE;

    uintptr_t sp = (uintptr_t)co->stack + co->stack_size;
    sp &= ~(uintptr_t)0xF;

    uint8_t* top =(uint8_t*) ((uintptr_t) sp - (uintptr_t) sizeof(void*));
    *(void**) top = coroutine_bootstrap;

    co->ctx.rsp = (void*)top; 
    co->ctx.r12 = NULL;
    co->ctx.r13 = NULL;
    co->ctx.r14 = NULL;
    co->ctx.r15 = NULL;
    co->ctx.rbp = NULL;
    co->ctx.rbx = NULL;

}

public void coroutine_yield(void){
    if(scheduler == NULL) return;

    if(scheduler->current_idx < 0){
        return;
    }
    else{
        coroutine_s *coro = &scheduler->coroutines[scheduler->current_idx]; 
        if(coro->status == CO_RUNNING) coro->status = CO_READY;
        __switch_context(&coro->ctx, &scheduler->ctx);
    }
}

public int coroutine_init(void){
    if(scheduler != NULL) return -1;
    scheduler = calloc(1, sizeof(scheduler_s)); // set to zero
    if(scheduler == NULL) return -1;
    scheduler->count = 0;
    scheduler->current_idx = -1;
    return 1;
}

public int coroutine_go(void(*func)(void)){
    if(scheduler == NULL) return -1;

    if(scheduler->count < COROUTINES_CAPACITY){
        create_coroutine(scheduler->count, func);
        scheduler->count +=1;
        return 0;
    }
    int i = choose_next_finished();
    if(i == -1) return -1;

    create_coroutine(i, func);
    return 0;
}

public void coroutine_run(void){
    if(scheduler == NULL) return; 
    scheduler_run();
}

public int coroutine_completed(){
    if(scheduler == NULL) return -1; 

    for(int i = 0; i < scheduler->count; ++i){
        if(scheduler->coroutines[i].status != CO_FINISHED) return 0;
    }
    return 1;
}
