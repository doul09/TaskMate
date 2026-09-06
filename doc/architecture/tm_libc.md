# 📚 Architecture Note — tm_libc

## Historical developments
`tm_libc` emerged to avoid full libc dependency on constrained targets and to control code size/behaviour. Over revisions, it gained formatted output support (`tm_snprintf`, `tm_vsnprintf`) and system logging helpers integrated with target-specific output backends.

After v0.28, `tm_libc` moved into the separated source tree and its include paths, Doxygen headers, and
identifier names were normalised. Formatter fixes accompanied stricter compiler warnings. In August
2026, formatting gained a cooperative-yield attempt around its shared lock so another thread could run
when the formatter was already in use.

## Current implementation
`interfaces/tm_options.h` selects either the TaskMate implementation or standard libc aliases at compile
time. In TaskMate mode:

- `tm_string_t` records whether text is in RAM or ROM, and AVR macros place constant strings in
  `PROGMEM`;
- `tm_strncpy()` performs the project's small bounded copy;
- `tm_snprintf()`, `tm_vsnprintf()`, `tm_printf()`, and `tm_vprintf()` share one static formatter state and
  support `%c`, `%s`, `%i`, `%x`, `%b`, `%%`, and one-digit zero padding;
- `tm_syslog()` forwards variadic arguments to the formatter.

The selected `hal/public/tmlibc.h` supplies character reads for RAM/ROM strings and character output.
On ATmega2560, output is buffered through USART and flushed when full or when a newline is written. The
implementation is intentionally much smaller than a conforming C stdio/string library.

## Well-built code and implementation weaknesses
### Strengths
- ROM-aware strings avoid copying diagnostic text into scarce AVR RAM.
- `tm_strncmp()` compares RAM/ROM descriptors without temporary copies, and `tm_strncpy()` handles
  null text, zero capacity, bounded copy, and termination when capacity is available.
- The supported formatting subset and its temporary storage are fixed; there is no heap allocation.
- Invalid padding exits through the common clean-up path instead of leaving the formatter lock set.

### Remaining weaknesses
- Formatting still uses one global buffer and a non-atomic byte lock. Contention yields only once
  and does not recheck ownership, so preemption, nesting, or ISR use can corrupt shared state.
- The buffer-capacity test can write before a non-null buffer of size 0, 1, or 2, underuses larger
  buffers, and returns stored length rather than standard `snprintf` would-have-written length.
- Numeric formatting reads 16-bit unsigned values despite `%i` and default variadic promotions. It
  lacks signed values, wider types, precision, multi-digit width, and bounded format/string indexes.
- The `TM_LIBC_CSTD` branch does not provide a complete compatible surface, notably for
  descriptor-based string calls and `tm_syslog`; only the TaskMate branch is exercised by the build.
- Logging has no levels, sink policy, delivery/back pressure result, or bounded-time guarantee, and
  its current HAL output may synchronously flush USART data.
