# 🏗️ Architecture Note — build

## Historical developments
TaskMate evolved from one Makefile into BSD `bmake` orchestration, focused `mk/*.mk` fragments, and
target-owned HAL fragments. autoCode and target validation became first-class build phases.

After tag `v0.28`, the source split made `HWT -> BOARD -> MCU -> ARCH` explicit. Commit `7ae12ca`
moved configuration and validated targets; `v0.29` (`9fc9513`) consolidated build rules.

Header allow-list parsing, warnings, and role-based variable names were then tightened around the
current AVR build pipeline. Commit `a2a7c65` added the integrated autoCode regression targets;
`e1d320a` made startup-header and startup-function lists separate generated inputs.

## Current implementation
Configuration declares the default `test1` target and a `test_noscli` composition for the same
Arduino Mega, ATmega2560, and AVR8 stack. The latter is intended to omit SCLI. System-wide and
target-owned module declarations are discovered separately and combined by autoCode.

The normal build checks tools and the hardware stack, regenerates autoCode, and verifies guarded
headers. It then collects dependencies, builds AVR firmware, and reports memory use and line counts.
Target artefacts, generated lists, logs, and stamps remain under `build/`.

Each selected target, board, MCU, and architecture fragment contributes its startup header and
function. autoCode emits their includes and calls in architecture-to-target initialization order.

Generic driver headers in `interfaces/` are explicit autoCode dependencies. The header checker scans
sources against `conf/header_allow.conf`, while compile-time guards protect critical headers.
Dedicated targets run autoCode by validation stage or as a complete black-box corpus, with an
ASan/UBSan build available for host-side memory and undefined-behaviour checks.

## Well-built code and implementation weaknesses
### Strengths
- Orchestration, discovery, hardware selection, checks, and utilities are separated by concern.
- Architecture, MCU, board, and target fragments contribute only their selected responsibilities.
- Missing target data, generated inputs, HAL selection, or guarded access fails before execution.
- AVR builds and host autoCode tests use broad warnings and explicit diagnostic reporting.

### Remaining weaknesses
- Invalid-target diagnostics still name the obsolete selector instead of the active target variable.
- Unsorted source and `*.rc` discovery can make ordering depend on filesystem enumeration.
- Build metadata varies with time and Git state, while tool versions are not pinned.
- Only one hardware stack exercises portability; the build also assumes BSD and Unix tooling.
