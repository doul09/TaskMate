# 🧩 Architecture Note — services

## Historical developments
Services were introduced after early core scheduling work to provide reusable system-level threads
(initially a message server and serial CLI) without mixing application code and kernel internals. The
message service was later removed.

After v0.28, services moved into `srcs/system/services` as part of the explicit system/user split. Their
headers and generated registration were adapted to the new include layout. More recently, the system
and CLI loops adopted the cooperative-yield syscall while waiting on software time counters, reducing
their deliberate spin time between polling cycles.

## Current implementation
`services_init.rc` registers two system threads at `RUN_SERVICE`. autoCode assigns each a fixed thread
record and stack:

- `system` reads the RTC and writes positioned LCD strings through syscalls, then cooperatively
  waits through the software time-counter syscalls;
- `scli` reads USART RX only through `sc_usartRead()`, assembles at most 63 bytes in a fixed local
  buffer, tokenizes the chunk, and dispatches the `driver`, `i2c`, and `thread` commands.

The system and SCLI both call `sc_coopYield()` while waiting. Resources are fixed at compile time,
with no heap allocation or service registry beyond the generated module database.

## Well-built code and implementation weaknesses
### Strengths
- Service threads, stacks, and SCLI buffers have fixed memory costs.
- SCLI uses fixed line/argument bounds, table-driven dispatch, RAM/ROM-aware comparisons, and
  explicit thread/driver list and life cycle commands through syscalls.
- USART RX returns explicit `err_codes_t` values across the syscall boundary. An empty RX buffer is
  normal polling state; other errors are reported through the error catalogue.
- Both services and the SCLI command handlers use syscalls rather than calling HAL drivers directly.

### Remaining weaknesses
- The system service currently ignores RTC/LCD syscall return codes, so display or clock failures are
  not reported or recovered.
- `tm_libc` still reaches target-specific string and output primitives through its documented
  transversal HAL backend. This is not a direct service-to-HAL bridge, but it remains target-coupled.
- SCLI polls the UART and processes each received chunk immediately instead of accumulating a
  terminated line. `scli_line_length` is unused, long input is split, and excess arguments are
  silently truncated.
- Repeated non-empty USART failures are logged on the same USART output path, so diagnostics may be
  unavailable when the peripheral itself is unusable.
