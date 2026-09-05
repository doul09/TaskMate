# 🏗️ Architecture Note — build

## Historical developments
TaskMate’s build system has evolved from a single entry Makefile into a layered BSD `bmake`
stack split by concern: global orchestration (`Makefile`), reusable infrastructure (`mk/*.mk`), and
target-dependent extension points (`srcs/hal/**/**_make.mk`). The current structure reflects two
parallel needs: (1) deterministic firmware build for embedded targets and (2) host-side project
tooling (autoCode generation, static checks, docs, editor helpers, and backup workflows).

A key step in this evolution is the integration of `autoCode` as a first-class build phase and the explicit `ARCH -> MCU -> BOARD` target validation path. Together, these choices move configuration errors and integration drift from runtime toward build time.

After v0.28, the source tree was separated into system, user, HAL, interfaces, and `tm_libc`
domains. Target selection moved to `user/target/<HWT>/hwt.mk`, which includes the matching board, MCU,
and architecture fragments. Header allow-list parsing moved to a dedicated AWK script, compiler warning
coverage was expanded, and Make variable names were normalised by role.

## Current implementation
The default `HWT=test1` selects `srcs/user/target/test1/hwt.mk`. That file adds target sources and
includes `board_make.mk`; the board fragment selects the ATmega2560 MCU, and the MCU fragment selects
the AVR8 architecture and compiler configuration. These fragments accumulate source roots, generated
HAL include lists, preprocessor target symbols, memory limits, programmer settings, and AVR compiler
flags.

The `all` target runs the architecture-boundary check, target-scoped
autoCode generation, dependency aggregation, AVR compilation/linking, memory-usage extraction, and
line counting. Objects, dependency files, maps, firmware output, autoCode configuration, and stamps live
under `build/`, with firmware artefacts separated by the selected hardware stack. Compile-time guards
protect critical headers, while `scripts/header_allow.awk` scans the source tree against
`conf/header_allow.conf` before compilation. The complete hardware stack is also checked against
`conf/hardware-targets.conf` before compilation.

## Well-built code and implementation weaknesses
### Strengths
- Build, source discovery, hardware selection, policy checks, utilities, and backup rules are
  separated into focused Make fragments.
- Compiler, linker, memory-report, and programmer rules now live under the AVR architecture, while
  MCU and board fragments contribute their own target values and source paths.
- A missing hardware target, incomplete hardware stack, missing GPIO declaration file, unavailable
  HAL implementation, or forbidden critical include fails before firmware execution.
- The AVR configuration enables a broad warning set, link-time optimisation, section garbage
  collection, dependency files, and explicit flash/RAM reporting.

### Remaining weaknesses
- The portable required-program list includes documentation, analysis, backup, and editor tools;
  their absence blocks even `clean`. Stamp caching also does not recheck tools until inputs change.
- Source and `*.rc` discovery use unsorted `find` output, so compile/link and module ordering can
  depend on filesystem enumeration even though error-file discovery is sorted.
- Build metadata includes dates, Git state, and a revision count, while tool versions are not
  pinned. `.BEGIN` also evaluates the ignored `srcs/interfaces/TaskMate_info.h` on every invocation,
  although a temporary-file comparison now avoids replacing it when its content is unchanged.
- The build remains tied to BSD `bmake`, Unix utilities, AVR tools, and machine-specific
  USB/programmer paths; no second hardware stack currently exercises the intended portability.
