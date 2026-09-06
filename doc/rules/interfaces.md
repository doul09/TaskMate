# 🔌 Interface Layer Rules

## Introduction

TaskMate separates **portable system contracts** from **hardware-specific implementations**.
To achieve this, the project defines a dedicated `interfaces/` directory containing **neutral interface
definitions** that are shared across all architectures, MCUs, and boards.

These interfaces describe **what the system expects**, while the Hardware Abstraction Layer (HAL)
implements **how the hardware fulfils those expectations**.

This separation keeps the architecture clear, prevents dependency inversions, and improves portability
when supporting new platforms.

Generic driver interface files use the `drv_<driver>.h` filename form. The following rules define how
the `interfaces/` layer must be used.

## Rule 1 — Interface independence

The `interfaces/` layer must not depend on any higher or lower system layer.

It may only include:

- basic C types
- minimal common headers

Examples: `stdint.h`, `stdbool.h`, `tm_types.h`

No dependency from `interfaces/` to `HAL`, `sysCall`, `services`, or `tasks` is allowed.

## Rule 2 — HAL dependency direction

The Hardware Abstraction Layer **may depend on** `interfaces/`, but must never depend on higher layers
 such as:

- `sysCall/`
- `services/`
- `tasks/`

The HAL implements the contracts defined in `interfaces/`, but it must remain completely unaware of
how higher layers use those interfaces.

## Rule 3 — System layer usage

Higher layers such as sysCall/ may depend on:

- `interfaces/`
- `HAL`

However, the dependency must **never go in the opposite direction**.

This ensures a clear architectural flow:

services / tasks
	↑
 sysCall
	↑
   HAL
    ↑
interfaces


Each layer may depend only on layers below it.

## Rule 4 — Hardware-specific interfaces

If an interface is specific to a particular architecture, MCU, or board, it **must not be** placed in
`interfaces/`.

Hardware-specific definitions belong in the **HAL layer**, where they remain isolated from portable
system contracts.

The `interfaces/` directory must only contain definitions that are meaningful across all supported
platforms.
