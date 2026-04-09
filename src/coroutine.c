#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#define public __attribute((visibility("default")))
#define private __attribute((visibility("hidden")))
#define COROUTINES_CAPACITY 10
typedef struct {
    void *rsp;
    void *rbp;
    void *rbx;
    void *r12;
    void *r13;
    void *r14;
    void *r15;
    void *rip;
} context_s;

typedef enum {
    CO_RUNNING,
    CO_FINISHED,
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

scheduler_s *scheduler;

private extern void __save_context(context_s* ctx);
private extern void __switch_context(context_s* from, context_s* to);
private extern void* __get_stack_base(int idx);

private int find_next_ready(void){
    for(int i = 0; i < scheduler->count; ++i){
        if(scheduler->coroutines[i].status == CO_READY && i != scheduler->current_idx) return i;
    }
    return -1;
}

private int find_next_finished(void){
    for(int i = 0; i < scheduler->count; ++i){
        if(scheduler->coroutines[i].status == CO_FINISHED) return i;
    }
    return -1;
}

private void create_coroutine(int i, void(*func)(void)){
    coroutine_s *co = &scheduler->coroutines[i];
    co->func = func;
    co->status = CO_READY;
    co->stack = __get_stack_base(i);
    co->ctx.r12 = NULL;
    co->ctx.r13 = NULL;
    co->ctx.r14 = NULL;
    co->ctx.r15 = NULL;
    co->ctx.rbp = NULL;
    co->ctx.rbx = NULL;
    co->ctx.rsp = co->stack;
}

public void coroutine_yield(void){
    if(scheduler->current_idx != -1)
        __save_context(&scheduler->coroutines[scheduler->current_idx].ctx); // assembly
    int next = find_next_ready();
    int current = scheduler->current_idx;
    if(next < 0) return;
    if(current >= 0) {
        if(scheduler->coroutines[current].status == CO_RUNNING) scheduler->coroutines[current].status = CO_READY;

        __switch_context(&scheduler->coroutines[current].ctx, &scheduler->coroutines[next].ctx);

        scheduler->coroutines[next].status = CO_RUNNING;
        scheduler->current_idx = next;
    }else{
        __switch_context(&scheduler->ctx, &scheduler->coroutines[next].ctx);
        scheduler->coroutines[next].status = CO_RUNNING;
        scheduler->current_idx = next;
    }
}

public int coroutine_init(void){
    if(scheduler != NULL) return -1;
    scheduler = malloc(sizeof(scheduler_s));
    scheduler->count = 0;
    scheduler->current_idx = -1;
    return 1;
}

public int coroutine_go(void(*func)(void)){
    if(scheduler->count >= COROUTINES_CAPACITY){
        int i = find_next_finished();
        if(i == -1) return -1;
        create_coroutine(i, func);
    }
    create_coroutine(scheduler->count, func);
    scheduler->count +=1;
    return 0;
}

public int coroutine_completed(void){
    for(int i = 0; i < scheduler->count; ++i){
        if(scheduler->coroutines[i].status != CO_FINISHED) return 0;
    }
    return 1;
}

public void coroutine_run(void){
    while(!coroutine_completed()){
        int next = find_next_ready();
        int current = scheduler->current_idx;
        if(next < 0) continue;
        if(current >= 0){
            if(scheduler->coroutines[current].status == CO_RUNNING) scheduler->coroutines[current].status = CO_READY;

            __switch_context(&scheduler->coroutines[current].ctx, &scheduler->coroutines[next].ctx);

            scheduler->coroutines[next].status = CO_RUNNING;
        }
        else {
            __switch_context(&scheduler->coroutines[current].ctx, &scheduler->coroutines[next].ctx);

            scheduler->coroutines[next].status = CO_RUNNING;
        }
    }
}
