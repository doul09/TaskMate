# 👨‍💻 Architecture Note — autoCode

## Historical developments
`autoCode` replaced manual allocation and include glue around `v0.10`. It gained `init.rc` parsing
in `v0.20`, then learned the architecture/MCU/board split in `v0.21`.

Revisions `v0.24` to `v0.26` added options, tagged replacement, temporary files, and diagnostics.
After tag `v0.28`, the system/user/HAL reorganisation changed inputs without changing its role.

Commit `bd42774` deferred destination replacement until parsing succeeds; `22c6781` then made
bounded line truncation explicit. Commit `79ac629` removed `FLOW` messages from firmware ROM.

Commits `bd9272d` and `70ef58e` added bounded diagnostic accumulation and file-error propagation.
Commit `a2a7c65` integrated black-box and sanitizer test targets for the host generator.

## Current implementation
`bmake` compiles `srcs/autoCode/` as a host tool and gives it selected system, HAL, and target input
lists. It:

- parses typed module entries, run levels, and optional driver I2C addresses;
- aggregates error declarations and their `FLOW`, `WARN`, `FAIL`, or `PANIC` level;
- reads selected HAL headers and logical GPIO declarations;
- rewrites tagged module, error, GPIO, and combined HAL regions.

Malformed records accumulate diagnostics up to a configured bound, then fail at phase boundaries.
Destinations are generated as registered temporary files, cleaned on failure, and compared and
replaced only after every input and required tag has passed validation.

Generated data fixes module records, stacks, contexts, run levels, driver callbacks, errors, GPIO
identifiers, and configured driver-interface includes. Black-box targets exercise command options,
catalogues, module inputs, tags, line bounds, stable replacement, and failed-generation isolation;
a separate target runs the same corpus with address and undefined-behaviour sanitizers.

## Well-built code and implementation weaknesses
### Strengths
- Required options and output tags are checked; invalid data prevents destination replacement.
- Module types, run levels, errors, addresses, and GPIO data are validated before compilation.
- Fixed generated records avoid runtime registration and dynamic allocation in the firmware.
- Generation and its host-side tests are build-integrated and deterministic on stable inputs.

### Remaining weaknesses
- Replacement has no rollback if a filesystem operation fails after an earlier rename.
- Tests do not inject real open, close, remove, or rename failures and provide no fuzz coverage.
- Input discovery order and host tool versions are not captured in a generation manifest.
