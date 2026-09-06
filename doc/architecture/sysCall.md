# 📞 Architecture Note — sysCall

## Historical developments
As TaskMate layered architecture matured, `sysCall` became the mediation layer between kernel/services/tasks and hardware-oriented implementation. It consolidated thread delay/yield APIs, error catalogue access, and GPIO logical operations.

After v0.28, GPIO calls were kept in the dedicated `sc_gpio` façade while the general syscall file was
adapted to the separated sysCore/HAL tree. In August 2026, an explicit cooperative-yield path was added:
a thread can mark itself yielded and request an early scheduler-timer interrupt instead of waiting only
for the next periodic preemption. USART RX was subsequently moved behind `sc_usartRead()`, removing
direct HAL access from the SCLI sources.

## Current implementation
The syscall layer currently has three small API groups:

- `sysCall.c` wraps the current thread's 16-bit software counter in an AVR atomic section, implements
  cooperative yield, exposes thread and driver life cycle operations, and mediates LCD, RTC, USART,
  and I2C operations;
- `sc_gpio.c` delegates logical set/get/toggle operations to the sysCore GPIO table;
- `error.c` owns the generated error catalogue and provides message lookup.

`sc_coopYield()` disables interrupts, marks the current module as yielded, reloads the scheduler timer
near its compare point, restores the interrupt state, and waits until the round-robin scheduler clears
the yielded bit when that thread is selected again. 

The LCD and RTC syscalls preserve the service -> sysCall -> HAL boundary and translate the driver's
last error into `err_codes_t`. `sc_usartRead()` validates its output pointer and translates a successful
HAL read to `ERR_NO_ERROR`.
When the driver rejects the read, the syscall returns its exact last error. The read and error snapshot
share one short AVR atomic section so the RX ISR cannot replace the error between those operations.

`sc_i2cScan()` first consumes the incremental `hal_i2cScan()` API into a static ten-address table.
It then marks drivers whose declared I2C address was not found as dead, and finally clears the
life-cycle and dead bits of dead drivers found during the scan so they return to `DRV_STATE_OFF`.

## Well-built code and implementation weaknesses
### Strengths
- Thread-counter access is protected against the timer ISR updating the same 16-bit state on AVR8.
- Thread information and life cycle APIs validate names/outputs, use RAM/ROM-aware comparison, and
  update run-level state in AVR atomic sections; the scheduler now skips stopped threads.
- Driver count, information, and life cycle calls use one generated control callback per driver
  instead of exposing private life cycle functions to services.
- The cooperative-yield mechanism reuses the existing scheduler interrupt and adds no dynamic state.

### Remaining weaknesses
- LCD and RTC operations expose only the subset currently used by the system service, and that service
  ignores their returned error codes.
- `sc_threadStart()` accepts unvalidated run levels and, when no saved level exists, records the
  supplied level without applying it to status. Repeated `sc_threadStop()` overwrites the saved
  level with `RL_RUN_NONE`.
- GPIO wrappers perform no signal bounds/configuration checks, and error lookup exposes only text
  rather than a complete error contract.
- Cooperative yield assumes task context and a running scheduler, but task/boot/ISR validity is not
  encoded in the API. The timer-load result is ignored and the resume spin has no timeout.
- Driver life cycle calls collapse all control failures to `bool`, while multi-call status reporting
  is not an atomic snapshot.
