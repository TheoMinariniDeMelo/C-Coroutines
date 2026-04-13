#include "../includes/coroutine.h"
#include <stdio.h>

void counter(){
    for (int i = 1; i <= 10; ++i){
        printf("%d\n", i);
        coroutine_yield();
    }
    return;
}

int main(void){
    coroutine_init();
    coroutine_go(counter);
    coroutine_go(counter);
    coroutine_run();
}
