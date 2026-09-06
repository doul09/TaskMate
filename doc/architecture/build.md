# 🏗️ Architecture Note — build

## Historical developments
TaskMate evolved from one Makefile into BSD `bmake` orchestration, focused `mk/*.mk` fragments, and
target-owned HAL fragments. autoCode and target validation became first-class build phases.

After tag `v0.28`, the source split made `HWT -> BOARD -> MCU -> ARCH` explicit. Commit `7ae12ca`
moved configuration and validated targets; `v0.29` (`9fc9513`) consolidated build rules.

Header allow-list parsing, warnings, and role-based variable names were then tightened around the
current AVR build pipeline.

## Current implementation
The default `test1` target selects Arduino Mega, ATmega2560, and AVR8 fragments. Together they
provide sources, symbols, generated HAL lists, limits, programmer settings, and compiler flags.

The normal build checks tools and the hardware stack, regenerates autoCode, and verifies guarded
headers. It then collects dependencies, builds AVR firmware, and reports memory use and line counts.
Target artefacts, generated lists, logs, and stamps remain under `build/`.

Generic driver headers in `interfaces/` are explicit autoCode dependencies. The header checker scans
sources against `conf/header_allow.conf`, while compile-time guards protect critical headers.

## Well-built code and implementation weaknesses
### Strengths
- Orchestration, discovery, hardware selection, checks, and utilities are separated by concern.
- Architecture, MCU, board, and target fragments contribute only their selected responsibilities.
- Missing target data, generated inputs, HAL selection, or guarded access fails before execution.
- AVR builds use broad warnings, LTO, section collection, dependencies, and flash/RAM reporting.

### Remaining weaknesses
- Missing optional tooling can block unrelated targets, including `clean`, until the stamp is valid.
- Unsorted source and `*.rc` discovery can make ordering depend on filesystem enumeration.
- Build metadata varies with time and Git state, while tool versions are not pinned.
- Only one hardware stack exercises portability; the build also assumes BSD and Unix tooling.
