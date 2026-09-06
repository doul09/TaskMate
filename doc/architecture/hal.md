# 🔧 Architecture Note — hal

## Historical developments
TaskMate started as AVR-centric code, then moved to a layered HAL model (notably around v0.21) with architecture (`hal/arch`), MCU (`hal/mcu`), and board (`hal/board`) separation. This progressively removed direct hardware code from `sysCore` and enabled cleaner portability planning (amd64 test path, arm target roadmap).

After v0.28, the repository completed a broader system/user/HAL splita nd introduced `hal/drivers` for reusable device drivers. Target-specific
GPIO wiring moved into `user/target`, while AVR context save/restore and scheduler-timer work moved
progressively into explicit assembly. Panic handling moved to the AVR architecture layer, interrupt
headers were clarified, and common register-bit operations were centralised.

## Current implementation
The only implemented hardware stack is `avr8 / atmega2560 / arduinoMega`, selected by the `test1`
hardware target. Its responsibilities are currently divided as follows:

- `hal/arch/avr8`: register context layout, stack-pointer access, interrupt/atomic primitives, context
  creation and restoration, architecture startup, and panic;
- `hal/mcu/atmega2560`: GPIO, I2C, USART, the 1 ms scheduler timer, the 10 ms software-counter timer,
  MCU startup, and AVR string/output support;
- `hal/board/arduinoMega`: the board startup hook;
- `hal/drivers`: the AMC2004 LCD and ZS042 RTC device implementations.

Generic driver contracts live in `interfaces/drv_*.h`; their implementations remain selected by the
target source tree and Make fragments. The headers remaining in `hal/public/` select concrete
architecture or MCU mechanisms and fail compilation when no implementation exists. autoCode generates
the combined target definition and startup-header lists. At runtime, boot calls architecture, MCU, and
board hooks, builds the GPIO mapping, and starts generated drivers by their configured run level.

## Well-built code and implementation weaknesses
### Strengths
- Context switching, interrupt control, timers, and MCU registers are concentrated in
  target-specific files rather than scattered through tasks or services.
- Public selection headers provide one include path per HAL capability and reject missing
  implementations at compile time.
- All six registered drivers implement the neutral contracts from `interfaces/` and expose one control
  entry point limited to the common run-level, life cycle, status, bit, and last-error contract; most
  operational I/O APIs reject calls while their driver is not running.
- Static generated driver registration and callback wiring keep firmware allocation deterministic.

### Remaining weaknesses
- Driver capability requirements remain implicit in the selected source set and `init.rc` names; a
  missing implementation is detected by compilation or linking rather than by a capability manifest.
- Architecture, MCU, and board startup hooks are empty. Boot special-cases run-level-zero USART and
  the scheduler timer, ignores life cycle returns, reports success unconditionally, and cannot unwind
  a partial hardware startup.
- I2C and USART contain unbounded polling or synchronous transmission. LCD/RTC translate I2C
  failures to a generic dependency error, while the `system` service ignores their results; address
  and timeout contracts remain incomplete.
- Initial AVR thread frames hard-code the third program-counter byte to zero, and the build
  deliberately caps usable flash at 64 KiB. Naked context-switch assembly has no automated ABI,
  behavioural, cycle, or stack-depth validation.
