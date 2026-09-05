# 🚨 Architecture Note — error

## Historical developments
TaskMate initially handled failures in a local, ad-hoc way inside each module. As the project grew and module count increased, this approach made diagnostics inconsistent and difficult to maintain. Around v0.23-v0.26, the project introduced a system-wide error model: modules declare symbolic error entries in `.err` files, `autoCode` validates and aggregates them at build time, and runtime code consumes a generated catalogue through `sysCall/error.*`. This transition moved error ownership from scattered string literals toward a centralized contract.

After v0.28, error declarations moved with their owners into the separated system and HAL trees. The
build now discovers and sorts the selected target's `*.err` files, while recent naming and include-path
cleanup kept the generated enum usable by HAL drivers, services, and sysCall code.

## Current implementation
Each non-comment `*.err` line declares a symbolic name, a quoted message, and a level. The four
levels are defined by TaskMate in `interfaces/error_level.h`; their declared semantics are:

- `FLOW`: normal interruption of control flow, handled by the thread;
- `WARN`: abnormal but recoverable interruption, handled by the thread and logged by the system;
- `FAIL`: component failure, handled by the system and recorded in persistent logs;
- `PANIC`: critical system problem, handled by the system through a controlled halt.

The build writes selected `*.err` paths to `build/<target>/files_error`; discovered target files are
sorted and the shared driver catalogue is appended explicitly. autoCode reads each listed catalogue,
uses the TaskMate `err_level_t` definition, rejects duplicate names and unknown level values, then
generates:

- `err_codes_t` and `ERROR_COUNT` in `interfaces/error_catalog.h`;
- ROM-backed messages and `err_item_t` entries in `system/sysCall/error.c`.

HAL drivers return a compact `hal_driver_state_t` and retain their precise `err_codes_t` value for
`DRV_CTRL_GETLASTERROR`. Selected syscalls, including USART read and I2C scan, return `err_codes_t`
directly to services. The runtime error API currently exposes only `err_getMessage(uint8_t)`, which
returns the generated string pointer for an in-range code and a null pointer otherwise. The level is
stored in the catalogue but is not exposed through a public accessor or used to select a runtime
response.

## Well-built code and implementation weaknesses
### Strengths
- Symbolic codes, messages, and levels originate from the `*.err` source catalogues.
- Duplicate names, malformed declarations, and invalid severity words are detected during
  generation.
- The firmware uses fixed-size enum/table data and ROM-backed text rather than runtime allocation.
- Message lookup checks its index before reading the generated array, and most HAL I/O operations
  reject use when their driver life cycle state is not running.

### Remaining weaknesses
- The only public lookup returns a message; callers cannot query level, error owner, or a
  prescribed recovery action through the API.
- Error signalling is fragmented between `err_codes_t`, `bool` syscall life cycle results, and
  `DRV_STATE_*` values. Success is not uniform, and boot plus the `system` service discard driver,
  RTC, and LCD return states.
- LCD and RTC now validate their public pointers and propagate failed I2C operations, but reduce the
  underlying I2C cause to `ERR_HAL_DRIVER_DEPENDENCY`; callers cannot retrieve a causal error chain.
- The generator exposes a 256-slot catalogue limit while lookup and several loops use 8-bit indexes;
  terminal-count handling and the ABI extension policy are undocumented.
- There is no structured runtime record containing context, occurrence count, timestamp, or
  originating module, and no tested escalation path from driver failure to safe state.
