# 🔌 Architecture Note — interface

## Historical developments
`interfaces/` emerged as TaskMate separated portable contracts from hardware implementations. It
collected common GPIO and pin definitions before becoming the dependency-neutral common root.

After tag `v0.28`, the tree split moved generated GPIO, module, run-level, string, and error
contracts there. Commits `66661d2` and `f6a6fa1` renamed headers and moved driver APIs here.

Commit `e804075` replaced thread-status defines with typed bits and tightened the representation.

## Current implementation
The layer has no HAL, sysCore, sysCall, service, task, or target-implementation includes. It owns:

- common GPIO types and generated logical signal identifiers;
- generic LCD, RTC, I2C, timer, and USART driver contracts;
- the terminal halt contract implemented by the selected architecture;
- generated error codes and shared error levels;
- string storage, options, bit helpers, run levels, and generated module limits.

Thread status now has typed bits for category, initialization, death, and cooperative yield. Driver
status and control remain a separate neutral protocol. Selected generated headers also hold target
counts and configured driver includes needed by system and HAL consumers.

## Well-built code and implementation weaknesses
### Strengths
- The layer remains dependency-neutral and deliberately transversal.
- System and HAL share compact contracts without exposing concrete target headers.
- Generated errors, signals, counts, and includes stay aligned with the selected target.
- Contracts add no runtime allocation or independent dispatch cost.

### Remaining weaknesses
- One broad module header mixes driver protocol, thread status, limits, and generated counts.
- Bit helpers use GNU extensions, and a local null definition overlaps standard C facilities.
- Generated thread status is emitted as raw integers despite its typed bit contract.
- Contracts do not express capabilities, ISR safety, or structured asynchronous errors.
