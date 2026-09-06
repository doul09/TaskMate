# 🧠 Architecture Note — sysCore

## Historical developments
`sysCore` owns static module data, scheduling policy, GPIO state, and software time.
MCU mechanisms moved to HAL, and context switching moved progressively into AVR assembly.

Boot code was removed: `TaskMate.c` owns pre-scheduler setup, while staged module startup runs
in the system service through syscalls.

## Current implementation
The module database contains generated driver records and four fixed thread control blocks, stacks,
saved contexts, run levels, status bits, and canaries. GPIO and software counters also remain
owned by sysCore.

The scheduler starts at the core run level with the system thread. Its 1 ms callback saves the AVR
context and selects a thread with a non-zero level no greater than the active level. The system
service advances that level after readiness checks. A separate 10 ms callback decrements software
counters; cooperative yield advances the next scheduling interrupt.

## Well-built code and implementation weaknesses
### Strengths
- Thread, stack, and driver records are static; firmware startup uses no heap.
- Context mechanics stay in HAL while selection and active run-level policy stay in sysCore.
- Run-level admission prevents later-stage services and tasks from running during startup.
- Stack canaries are checked on both sides of every context switch.

### Remaining weaknesses
- Eligible threads are equal round-robin peers; dead and initialized bits do not affect selection.
- There is no priority, blocking, deadline, idle-thread, watchdog, or overrun policy.
- Module index access lacks bounds checks, and shared current-thread state has no explicit contract.
- Startup remains split across top-level and service code and cannot unwind partial initialization.
