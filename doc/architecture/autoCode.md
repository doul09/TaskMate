# 👨‍💻 Architecture Note — autoCode

## Historical developments
`autoCode` replaced manual allocation and include glue with build-time generation. It later gained
typed `init.rc` parsing, target selection, error catalogues, GPIO signals, tagged replacements, and
stricter diagnostics.

The system/user/HAL reorganisation changed its inputs but retained the same role: selected target
files are the source of truth for static firmware composition.

## Current implementation
`bmake` compiles `srcs/autoCode/` as a host tool and gives it target-scoped input lists. It:

- parses typed module entries, run levels, and optional driver I2C addresses;
- aggregates error declarations and their `FLOW`, `WARN`, `FAIL`, or `PANIC` level;
- reads selected HAL headers and logical GPIO declarations;
- rewrites tagged module, error, GPIO, and combined HAL regions.

Each destination is regenerated through a temporary file and replaced only when changed. Generated
data fixes module counts, records, stacks, contexts, names, run levels, status, driver callbacks,
addresses, error codes, GPIO identifiers, and configured driver-interface includes.

## Well-built code and implementation weaknesses
### Strengths
- Required options and output tags are checked; invalid data stops generation.
- Module types, run levels, errors, addresses, and GPIO data are validated before compilation.
- Fixed generated records avoid runtime registration and dynamic allocation in the firmware.
- Generation is build-integrated, logged, and stable when its inputs do not change.

### Remaining weaknesses
- Destinations are replaced independently, without rollback or complete file-operation checks.
- Fixed input-line bounds lack explicit overlong-line detection and can split one physical line.
- There is no parser corpus, failure-injection suite, or manifest of input and tool versions.
