# 🧠 Architecture Note — sysCore

## Historical developments
`sysCore` grew from a single-file prototype into module data, scheduler, GPIO, and software-time
ownership. MCU mechanisms moved to HAL, while context switching progressed into AVR assembly.

After tag `v0.28`, `TaskMate.c` became the top-level entry point; `8697df7` hardened canary checks.
Commits `c843372` and `35f329d` removed boot code in favour of top-level and service startup.

Commit `db59169` made the scheduler admit only threads at or below the active run level.

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
