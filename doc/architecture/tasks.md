# 🧵 Architecture Note — tasks

## Historical developments
User tasks started as direct test routines in early TaskMate revisions and were progressively normalized into module-style entries (`tasks_init.rc`) managed by autoCode. This enabled deterministic static thread creation and reduced manual startup code.

After v0.28, user code moved out of the system tree into `srcs/user/tasks`, while hardware-target data
moved into `srcs/user/target/test1`. Task declarations became part of the target's typed
`test1_init.rc`, and LED wiring was moved into target configuration. The syscall layer later gained
cooperative yield, although the two example tasks still use their original polling delay loops.

## Current implementation
`test1_init.rc` registers `task1` and `task2` as `RUN_USER` threads. autoCode creates their control
blocks, 256-element `hal_stack_word_t` stacks, initial AVR contexts, names, status bytes, and
function pointers. On the current AVR8 target each stack element is one byte. The scheduler includes
both in the same round-robin set as the service threads.

Each example task loops forever. In each loop it toggles its target-defined logical LED through
`sc_gpio`, sets its current thread's software counter to 50, and busy-waits until the 10 ms counter
reaches zero. Periodic scheduler interrupts still preempt the task during that wait.

## Well-built code and implementation weaknesses
### Strengths
- The two tasks are small, deterministic examples with no direct HAL or register access.
- Generated logical GPIO calls demonstrate the intended user -> sysCall -> sysCore -> HAL direction.
- Generated registration and fixed stacks avoid runtime allocation and module registration.
- Their identical, bounded loop bodies make scheduler and GPIO behaviour easy to compare on hardware.

### Remaining weaknesses
- Period, deadline, priority, stack need, and worst-case execution time are not declared or checked;
  the fixed 256-byte stack is assigned without per-task sizing evidence.
- Run levels currently provide only runnable/stopped gating for threads; `RUN_USER` has no
  scheduling policy distinct from service threads.
- Both files retain an unused global `task*_msg_channel` and an unused `tm_stdio.h` include from the
  removed startup-message code.
- There are no task watchdog, overrun, failure-reporting, or fault-containment hooks.
