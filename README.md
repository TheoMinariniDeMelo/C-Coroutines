# ccoroutines

Didactic implementation of cooperative coroutines in C, with context switching in NASM assembly for `x86_64`.

The project provides a simple round-robin scheduler: each coroutine runs until it calls `coroutine_yield()`, returning control to the scheduler, which then picks the next ready coroutine.

## Overview

- Cooperative scheduler, with no preemption.
- Manual context switching by saving callee-saved registers.
- Separate stacks for each coroutine.
- Small C API to initialize, register, and run coroutines.

## How it works

Each coroutine has:

- CPU context (`rsp`, `rbp`, `rbx`, `r12`-`r15`);
- a pointer to the function it will execute;
- its own stack;
- a state (`CO_READY`, `CO_RUNNING`, `CO_FINISHED`).

The scheduler keeps a fixed list of coroutines and switches between them.
When a coroutine calls `coroutine_yield()`, the current context is saved and control returns to the scheduler.
When the coroutine function finishes, it is marked as completed.

## Project structure

```text
.
|-- includes/
|   `-- coroutine.h
`-- src/
    |-- coroutine.c
    |-- coroutine-x86_64.asm
    `-- main.c
```

- `includes/coroutine.h`: public API.
- `src/coroutine.c`: scheduler, coroutine creation, and state handling.
- `src/coroutine-x86_64.asm`: context switching and stack storage.
- `src/main.c`: minimal usage example.

## Public API

```c
void coroutine_yield(void);
int coroutine_init(void);
int coroutine_go(void (*func)(void));
void coroutine_run(void);
int coroutine_completed(void);
```

- `coroutine_init()`: initializes the scheduler.
- `coroutine_go(func)`: registers a new coroutine.
- `coroutine_run()`: runs the scheduler until all coroutines finish.
- `coroutine_yield()`: voluntarily returns control to the scheduler.
- `coroutine_completed()`: reports whether all coroutines have finished.

## Example

```c
#include "coroutine.h"
#include <stdio.h>

void counter(void) {
    for (int i = 1; i <= 3; ++i) {
        printf("%d\n", i);
        coroutine_yield();
    }
}

int main(void) {
    coroutine_init();
    coroutine_go(counter);
    coroutine_go(counter);
    coroutine_run();
    return 0;
}
```

Expected output:

```text
1
1
2
2
3
3
```

## Build

Dependencies:

- `gcc`
- `nasm`

Commands validated on Linux `x86_64`:

```bash
mkdir -p obj
nasm -f elf64 src/coroutine-x86_64.asm -o obj/coroutine.o
gcc -Wall -Wextra -Iincludes src/main.c src/coroutine.c obj/coroutine.o -o obj/ccoroutines-demo
./obj/ccoroutines-demo
```

Note: `Makefile` and `build.lua` still appear to be under development. In the current state of the repository, the most reliable path is to build with the commands above.

## Current limitations

- Implementation focused on `Linux x86_64`.
- Fixed capacity of `10` coroutines (`COROUTINES_CAPACITY`).
- Fixed stack size of `4 KiB` per coroutine (`MAX_STACK_SIZE`).
- Function signature limited to `void (*)(void)`.
- Strictly cooperative model: if a coroutine does not call `coroutine_yield()`, it monopolizes execution.

## Purpose

This project is useful for studying:

- low-level context switching;
- cooperative scheduler implementation;
- C and assembly integration;
- coroutine fundamentals without relying on an external runtime.
