# 📞 Architecture Note — sysCall

## Historical developments
`sysCall` became the task-visible boundary for kernel state, logical GPIO, and hardware operations.
Cooperative yield and USART RX later removed direct HAL access from services.

The implementation is now split by responsibility and includes run-level startup coordination,
thread initialization, RTC date access, and a system panic entry point.

## Current implementation
Four focused groups provide the boundary:

- HAL calls mediate driver life cycle, LCD, RTC, I2C discovery, and USART RX;
- module calls mediate run levels, threads, counters, readiness, and cooperative yield;
- GPIO calls delegate logical signal operations to sysCore;
- error calls expose generated messages and controlled panic.

Run-level changes are atomic, monotonic, and bounded. Driver stages use generated callbacks.
Readiness requires matching drivers to run and matching threads to declare initialization. The
scheduler uses the resulting active level to admit threads.

RTC calls validate pointers, translate errors, and keep one startup-time snapshot. The bounded
I2C scan reconciles declared devices only after a complete, non-overflowing discovery pass.

## Well-built code and implementation weaknesses
### Strengths
- AVR-shared counters, run levels, and thread status updates use short atomic sections.
- Module lookup and metadata calls validate public inputs and use RAM/ROM-aware names.
- Services reach drivers through typed syscalls with explicit error translation.
- I2C discovery uses fixed storage and preserves state when the scan result is incomplete.

### Remaining weaknesses
- Driver-stage start returns no result and discards individual initialization and start failures.
- Thread readiness is caller-declared; start still accepts an unchecked initial run level.
- The RTC startup snapshot has no validity state when its source read fails.
- GPIO and cooperative-yield APIs lack bounds, context, timeout, and ISR contracts.
