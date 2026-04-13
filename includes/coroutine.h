void coroutine_yield(void);
int coroutine_init(void);
int coroutine_go (void(*)(void));
void coroutine_run(void);
int coroutine_completed();
