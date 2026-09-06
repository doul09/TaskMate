# 🧵 Architecture Note — tasks

## Historical developments
User tasks evolved from direct test routines into autoCode-managed modules with fixed records and
stacks. User code and target wiring were separated into dedicated source trees.

Tasks now participate in staged startup by declaring initialization through sysCall at entry.

## Current implementation
The `test1` target registers two user-level tasks. autoCode creates their fixed 256-byte AVR stacks,
initial contexts, names, status, saved run levels, and entry callbacks.

The scheduler excludes them until the system service advances to the user run level. Each task then
marks itself initialized, toggles a target-defined logical LED, loads a 500 ms software delay, and
busy-waits while periodic scheduler interrupts continue to preempt it.

## Well-built code and implementation weaknesses
### Strengths
- Both tasks are deterministic examples with no direct HAL or register access.
- Logical GPIO demonstrates the intended task -> sysCall -> sysCore -> HAL path.
- Generated registration and fixed stacks avoid runtime allocation.
- Initialization acknowledgement integrates tasks into staged system startup.

### Remaining weaknesses
- Period, deadline, priority, stack need, and worst-case execution time are not declared or checked.
- User level gates activation but gives no distinct scheduling policy afterward.
- Busy-wait delays consume each scheduled slice instead of yielding cooperatively.
- Unused message-channel state and `tm_stdio` dependencies remain from removed startup output.
