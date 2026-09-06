# Documentation rules audit

Audit date: 2026-09-06.

## Scope and classification

The files formerly present in `doc/rules/*.md` were checked against the current
source tree, build files, and the architecture notes. A file belongs in
`doc/rules/` only when it gives prescriptive, durable constraints to contributors.
Descriptions of the current implementation, historical notes, tutorials, and
design assessments belong elsewhere in `doc/`.

| Document | Classification | Action |
|---|---|---|
| `rules/style.md` | Contributor rules | Kept. |
| `rules/interfaces.md` | Dependency rules | Kept. |
| `rules/TaskMate_prefixes.md` | Naming rules | Kept and corrected. |
| `rules/make_variable_prefixes.md` | Build naming rules | Kept. |
| `rules/arch_boundary_enforcement.md` | Implementation description | Moved to `arch_boundary_enforcement.md`. |
| `rules/autoCode.md` | Obsolete implementation overview | Removed. |
| `rules/portability.md` | Obsolete design overview and tutorial | Removed. |

## Obsolete documentation removed

### autoCode overview

The removed overview duplicated `architecture/autoCode.md` but described old
paths such as `hal/arch/<arch_name>/drivers_init.rc`, `services/services_init.rc`,
and generated outputs below unqualified `sysCore/`, `sysCall/`, and `hal/`
directories. The architecture note records the current `srcs/` layout, target
configuration, `*.rc`, `*.err`, `*.gpio`, generated regions, and known
limitations. Maintaining a second, less precise overview in the rules directory
would invite drift.

### Portability overview and target tutorial

The removed document mixed design explanation, GPIO architecture, and a target
addition tutorial rather than imposing contributor rules. Its examples used the
old `src/` tree, selected hardware with `ARCH`, `MCU`, and `BOARD` command-line
variables, ran GNU `make`, placed target wiring in board startup, and named GPIO
APIs that no longer exist. The current build selects `HWT`, target wiring lives
under `srcs/user/target/<target>/`, tasks call `sc_gpio_*`, and current design and
limitations are covered by `architecture/build.md`, `architecture/gpio.md`, and
`architecture/hal.md`.

## Documentation moved or corrected

`arch_boundary_enforcement.md` accurately describes a current build mechanism:
the allow-list check and compile-time guards are implemented by the build files,
AWK checker, configuration, and guarded headers. Because it explains how the
mechanism works rather than defining a standalone set of contributor rules, it
now lives directly under `doc/`.

The `sc_` entry in `rules/TaskMate_prefixes.md` previously called the syscall API
a security and privilege boundary. TaskMate provides an API and architectural
boundary, but no privilege or memory isolation, so the rule now states that
limitation explicitly.

## Remaining rule set

The remaining files under `doc/rules/` are intentionally prescriptive:

- `style.md` defines formatting, naming, file structure, embedded-C, generated
  code, and build-script conventions;
- `interfaces.md` defines allowed dependency directions for the neutral
  interface layer;
- `TaskMate_prefixes.md` reserves prefixes for actual architectural concepts;
- `make_variable_prefixes.md` defines the semantic naming scheme for build
  variables.

Future current-state descriptions should update the relevant architecture note
instead of adding a parallel overview to `doc/rules/`.
