# 🔧 Architecture Note — hal

## Historical developments
TaskMate began as AVR-centric code, then `v0.21` separated architecture, MCU, and board ownership.
This progressively removed hardware detail from sysCore and clarified target-specific boundaries.

After tag `v0.28`, the repository introduced reusable external drivers and moved target GPIO wiring.
Commit `b201809` completed the HAL/GPIO refactor; tag `v0.29` recorded the Make and HAL baseline.

Generic driver contracts later moved to `interfaces/`; HAL retained selection and implementation.

## Current implementation
The only implemented stack is `avr8 / atmega2560 / arduinoMega`, selected by `test1`:

- architecture code owns context, stack, interrupt, atomic, startup, and panic mechanisms;
- MCU code owns GPIO, I2C, USART, timers, startup, and AVR text/output support;
- board code provides the Arduino Mega startup hook;
- reusable drivers implement the AMC2004 LCD and ZS042 RTC contracts.

Public HAL headers select architecture or MCU mechanisms. Generic driver APIs come from neutral
interfaces and are bound by target sources and generated includes. Before scheduling, generated
calls initialize architecture, MCU, board, then target hooks before logical GPIO initialization.
USART still starts earlier as the boot-log path. Once scheduled, the system service starts other
drivers by run level through syscalls and checks their running state before advancing.

## Well-built code and implementation weaknesses
### Strengths
- CPU context, interrupts, timers, and registers remain inside target-specific code.
- Public selectors reject unavailable mechanisms at compile time.
- The build-selected startup order is explicit, generated, and covered by host tests.
- Registered drivers share one bounded life-cycle and status contract.

### Remaining weaknesses
- Driver capability requirements remain implicit in selected sources and `init.rc` names.
- Startup hooks are empty; USART and scheduler timers still follow special initialization paths.
- Start requests discard driver results, and startup cannot unwind a partial hardware state.
- Polling, synchronous I/O, AVR frame assumptions, and ABI validation remain hardware risks.
