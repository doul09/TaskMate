# 🔌 Architecture Note — interface

## Historical developments
The dedicated `interfaces/` layer appeared as TaskMate matured toward portability and explicit dependency rules. Earlier hardware-oriented code was gradually refactored to isolate portable logical contracts (e.g., GPIO logical signals, common pin modes) from concrete HAL implementation details.

After v0.28, the larger system/user/HAL separation made `interfaces/` the common dependency root for the
new tree. GPIO signal IDs and module/run-level counts became generated there, directory-based header
guards were applied consistently, and the register-bit macros used by AVR drivers were moved into
`interfaces/macros.h` and subsequently hardened and renamed.

## Current implementation
`interfaces/` contains no HAL, sysCore, sysCall, service, or task includes. It currently provides:

- common GPIO modes/pulls and the target-generated logical signal enum;
- the generated error enum and shared `FLOW`/`WARN`/`FAIL`/`PANIC` level type;
- shared string-storage types and build-time libc/debug options;
- general bit/string helper macros and a local null definition;
- generated module counts, fixed run-level definitions, and shared driver/module limits used by
  autoCode and selected core files.

Most headers are type, enum, constant, or macro contracts. The build guarded-header mechanism is
currently applied to selected HAL context/interrupt/stack headers and generated sysCore headers, not
to the interface headers themselves. This makes the directory both a portability-contract layer and
a home for a small amount of generated kernel configuration.

## Well-built code and implementation weaknesses
### Strengths
- `interfaces/` remains a deliberately transversal, dependency-neutral layer with no HAL, sysCore,
  sysCall, service, task, or target-implementation includes.
- Shared GPIO, error, string-storage, run-level, and driver-control definitions let system and HAL
  code agree without duplicating representations.
- Generated error, GPIO, and module-count values keep selected-target metadata consistent with
  runtime tables.
- Contracts use compact data with no runtime allocation or dispatch cost of their own.

### Remaining weaknesses
- `modules_define.h` combines the public driver protocol, thread-status layout, generator limits,
  stack sizing, and generated kernel counts, coupling separate consumers to one broad contract.
- `macros.h` uses GNU `__typeof__` and register-oriented size dispatch, while `define.h` provides
  its own `NULL`; these choices reduce compiler neutrality and overlap standard C facilities.
- Driver states, module types, and run levels remain compact integer values. Generated thread
  status values are still emitted as raw integer literals and the shared control-data union cannot
  encode which field is valid for a command.
- Contracts remain incomplete for timers, serial transport, scheduling context, ISR safety,
  optional HAL capabilities, and structured error handling.
