# 👨‍💻 Architecture Note — autoCode

## Historical developments
`autoCode` replaced earlier manual include/alloc glue code around v0.10, then evolved strongly in v0.20+ with `init.rc` parsing and in v0.21+ with arch/mcu/board split support. In v0.24-v0.26, reliability features were reinforced: option parsing, per-file temporary replacement, generated-tag discipline, and tighter diagnostics. This history shows a deliberate move from ad-hoc startup wiring toward a build-time source-of-truth model.

After v0.28, the system/user/HAL directory reorganisation changed the generator inputs. The selected
hardware target now provides ordered header and source lists through the build system, while
`signals.gpio`, typed `*.rc` entries, and the global `*.err` catalog feed additional generated regions.

## Current implementation
`bmake` compiles `srcs/autoCode/` as a host program with Clang and writes a target-specific
configuration file containing the paths to its input lists and selected GPIO file. The program then:

- parses the selected `*.rc` files, whose module entries contain separate `-type <data>` and
  `-run <data>` pairs plus an optional driver-only `-i2c <address>` pair, into fixed-size driver and
  thread databases;
- aggregates `*.err` declarations and their `FLOW`, `WARN`, `FAIL`, or `PANIC` level;
- reads the selected HAL/target header lists and `signals.gpio`;
- rewrites the tagged regions in module, error, GPIO, and combined HAL include files.

Each destination is copied to a `.tmp` file, regenerated, compared with the existing file, and replaced
only when its content changed. A target-scoped stamp makes generation a prerequisite of dependency
collection, compilation, and linking. The generated data fixes module counts, stacks, function tables,
module run-level fields, generic driver address metadata populated by the current `-i2c` option,
error codes, logical GPIO identifiers, and the `interfaces/drv_<name>.h` includes for configured drivers
at build time.

## Well-built code and implementation weaknesses
### Strengths
- Required options and output tags are counted; missing, duplicated, or unknown declarations stop
  generation.
- Module types, run levels, same-type duplicate names, error severities, and GPIO line token counts
  receive explicit validation before the firmware is compiled.
- Generated records include fixed thread contexts, saved run levels, driver control callbacks, and
  ROM-backed names and generic driver address metadata, avoiding runtime module registration and
  dynamic allocation.
- Generation is integrated into the dependency graph, produces a reviewable log, and preserves an
  existing destination when its generated content is unchanged.

### Remaining weaknesses
- Replacement is performed one destination at a time. It removes the old file before renaming the
  temporary file, ignores `remove()`/`rename()` failures, and cannot roll back earlier replacements.
- Input lines still use a fixed 256-byte buffer without an explicit overlong-line check. Token
  pointers are grown with host-side `realloc()` for each token, so allocation failure terminates the
  generator and a long physical line can be parsed as multiple fragments.
- There is no automated valid/invalid corpus, boundary test suite, failure-injection test, or
  manifest recording input hashes and generator/tool versions.
