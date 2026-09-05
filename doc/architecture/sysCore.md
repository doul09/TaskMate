# 🧠 Architecture Note — sysCore

## Historical developments
`sysCore` grew from a single-file prototype into the kernel heart: module database, run-level structures, scheduler, and software time counters. A major refactor moved MCU-specific code out of core into HAL, while keeping core ownership of policy (thread switching, module life cycle metadata).

After v0.28, `TaskMate.c` moved above sysCore and the repository gained explicit system, user, HAL, and
interface boundaries. Thread/module pointers and include paths were corrected during that transition.
The AVR context switch was then moved progressively into assembly, stack panic handling moved to the
architecture layer, and the scheduler gained a cooperative-yield trigger in addition to its periodic
preemption.

## Current implementation
`boot.c` starts the diagnostic USART and initialises generated driver/thread records,
calls architecture/MCU/board startup hooks, wires GPIO signals, and starts every generated driver in
ascending configured run level. It runs the I2C discovery syscall immediately after starting the I2C
driver, before higher-run-level dependent drivers start. `modules.c` owns a static database
containing generic driver address metadata and control callbacks, four thread control blocks, fixed
stacks, saved stack pointers, status bytes, and stack canaries.

The scheduler installs a callback into the 1 ms HAL timer, starts with thread zero, and performs strict
round-robin selection across all generated threads. The naked AVR timer ISR saves context, gives the
saved stack pointer to `tm_schedulerRR()`, installs the returned pointer, and restores context. A separate
10 ms timer callback decrements every non-zero software counter. Cooperative yield accelerates the next
scheduler interrupt; it does not remove a thread from the round-robin set. GPIO mapping/state and the
module database also live in sysCore; run levels are stored directly in module status fields.

## Well-built code and implementation weaknesses
### Strengths
- Thread control blocks, stacks, and driver records are statically allocated. Driver life-cycle
  state is file-local in HAL, and no runtime heap is used.
- Context-switch mechanism is delegated to HAL/AVR code while selection policy remains in sysCore.
- The scheduler now selects only threads with a non-zero run level, clears cooperative-yield state
  on resume, and panics explicitly when no runnable thread exists.
- Stack canaries are initialised for every thread and checked on both sides of every context switch.
- Separate 1 ms scheduling and 10 ms delay counters provide simple, predictable timing primitives.

### Remaining weaknesses
- Non-zero thread run levels have identical round-robin eligibility; dead/type bits do not affect
  selection, and there are no priority, readiness, blocking, deadline, idle-thread, or overrun
  semantics. Stopping every thread ends in panic rather than an idle state.
- Module pointer and current-thread getters/setters do not validate indexes. `thread_current` is
  shared with the scheduler ISR but is neither volatile nor governed by a documented access
  contract.
- Boot special-cases USART, invokes the higher `sc_i2cScan()` layer from sysCore, ignores every life
  cycle/scan result, logs success unconditionally, and cannot unwind a partial startup.
  Scheduler and software-counter timer setup also ignore callback/control failures.
- Stack canaries detect only boundary corruption at a context switch; there is no stack high-water
  measurement or per-thread sizing evidence. The target-specific RTC/LCD experimental code now runs
  inside the `system` service and directly crosses from services to HAL.
