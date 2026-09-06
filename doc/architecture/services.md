# 🧩 Architecture Note — services

## Historical developments
Services introduced reusable system threads above the kernel, initially including a message service
and serial CLI. The message service was later removed as the system service took its role.

After tag `v0.28`, services moved under `srcs/system/services`. Commit `5109e98` put SCLI USART RX
behind sysCall, and later cooperative yield shortened deliberate polling waits.

Commits `c843372` and `35f329d` moved boot work to `TaskMate.c`, then staged startup to `system`.

## Current implementation
autoCode registers a core-level `system` thread and a service-level `scli` thread with fixed stacks.
Each thread declares itself initialized through sysCall when its entry begins.

The system service starts drivers one run level at a time, triggers I2C discovery, stores the RTC
startup date, then waits for driver and thread readiness before enabling the next level. It reads
the RTC, updates the LCD, and cooperatively waits on its software counter.

SCLI reads USART through sysCall into a fixed buffer and dispatches `date`, `driver`, `i2c`, and
`thread`. The date command reads or updates RTC fields and can display the captured startup date.

## Well-built code and implementation weaknesses
### Strengths
- Service records, stacks, command tables, and buffers have fixed memory costs.
- Startup follows explicit core, driver, service, and user stages with bounded readiness rounds.
- Both services and all command handlers preserve the service -> sysCall boundary.
- RTC command errors are translated through the generated error catalogue.

### Remaining weaknesses
- Startup discards I2C-scan, RTC-snapshot, and individual driver start results.
- Thread readiness is self-declared, with no richer health or dependency state.
- The display loop still ignores RTC/LCD errors and provides no recovery policy.
- SCLI processes RX chunks rather than complete lines and silently truncates excess arguments.
