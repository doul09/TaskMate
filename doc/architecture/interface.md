# 🔌 Architecture Note — interface

## Historical developments
`interfaces/` became the dependency-neutral home for portable contracts shared by system and HAL.
The system/user/HAL split moved common GPIO, error, string, module, and run-level definitions there.

Generic driver headers and normalized TaskMate names later removed concrete driver-header coupling.

## Current implementation
The layer has no HAL, sysCore, sysCall, service, task, or target-implementation includes. It owns:

- common GPIO types and generated logical signal identifiers;
- generic LCD, RTC, I2C, timer, and USART driver contracts;
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
