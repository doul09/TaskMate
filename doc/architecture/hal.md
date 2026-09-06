# 🔧 Architecture Note — hal

## Historical developments
TaskMate moved from AVR-centric code to distinct architecture, MCU, board, and external-driver
layers. Context switching, panic, target wiring, and driver contracts were progressively assigned to
explicit owners.

Generic driver contracts later moved to `interfaces/`; HAL retains selection and implementation.

## Current implementation
The only implemented stack is `avr8 / atmega2560 / arduinoMega`, selected by `test1`:

- architecture code owns context, stack, interrupt, atomic, startup, and panic mechanisms;
- MCU code owns GPIO, I2C, USART, timers, startup, and AVR text/output support;
- board code provides the Arduino Mega startup hook;
- reusable drivers implement the AMC2004 LCD and ZS042 RTC contracts.

Public HAL headers select architecture or MCU mechanisms. Generic driver APIs come from neutral
interfaces and are bound by target sources and generated includes. Before scheduling, system startup
initializes USART, HAL hooks, and GPIO. Once scheduled, the system service starts other drivers by
run level through syscalls and checks their running state before advancing.

## Well-built code and implementation weaknesses
### Strengths
- CPU context, interrupts, timers, and registers remain inside target-specific code.
- Public selectors reject unavailable mechanisms at compile time.
- Registered drivers share one bounded life-cycle and status contract.
- Static generated registration keeps allocation and dispatch deterministic.

### Remaining weaknesses
- Driver capability requirements remain implicit in selected sources and `init.rc` names.
- Startup hooks are empty; USART and the scheduler timer still follow special pre-service paths.
- Start requests discard driver results, and startup cannot unwind a partial hardware state.
- Polling, synchronous I/O, AVR frame assumptions, and ABI validation remain hardware risks.
