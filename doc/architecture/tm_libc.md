# 📚 Architecture Note — tm_libc

## Historical developments
`tm_libc` was introduced to bound code size and behaviour on constrained targets. It gained compact
formatting, RAM/ROM-aware strings, and logging through target-specific output backends.

After tag `v0.28`, it became a transversal layer in the separated tree. Commit `ec88d98` corrected
formatter behaviour; later revisions added a cooperative-yield attempt for contention.

Tag `v0.29` marks the baseline before the current syscall and startup refactors.

## Current implementation
Build options select either TaskMate implementations or partial standard-library aliases. TaskMate
mode provides bounded copy and comparison plus compact print functions supporting characters,
strings, integers, hexadecimal, binary, percent, and one-digit zero padding.

Text descriptors distinguish RAM from AVR program memory. Formatting and logging share fixed static
state. The public HAL backend reads stored text and buffers USART output until full or newline.

## Well-built code and implementation weaknesses
### Strengths
- Program-memory strings reduce scarce AVR RAM use.
- Bounded string operations handle null text, capacity, and termination explicitly.
- Formatting uses a fixed feature set and temporary storage with no heap.
- Invalid padding leaves through the common formatter cleanup path.

### Remaining weaknesses
- Formatting uses shared state and a non-atomic lock; one yield does not guarantee ownership.
- Small buffer capacities can be mishandled, and return length differs from standard `snprintf`.
- Numeric variadic handling is narrow and lacks signed, wide, precision, and robust bounds support.
- The standard-libc branch is incomplete, while logging lacks level, sink, and timing policy.
