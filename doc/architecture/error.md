# 🚨 Architecture Note — error

## Historical developments
TaskMate replaced local ad-hoc error strings with module-owned `*.err` declarations between `v0.23`
and `v0.26`. autoCode then aggregated them into one symbolic catalogue for HAL and system code.

After tag `v0.28`, declarations moved with their owners while their generated contract stayed in
`interfaces/`. Commits `7ee2725` and `08771cb` established the current four-level definition.

Commit `c8d3d21` repaired the error-catalogue tag and preserved generation as the source of truth.

## Current implementation
Each declaration contains a symbolic name, quoted message, and one of four levels:

- `FLOW`: normal control-flow interruption handled by the thread;
- `WARN`: recoverable abnormal condition handled and logged by the thread;
- `FAIL`: component failure handled by the system and intended for persistent logging;
- `PANIC`: critical system condition requiring a controlled halt.

The build sorts selected catalogues before generation. Firmware code can resolve an error message
through sysCall. System code can also request panic there without including HAL panic.

## Well-built code and implementation weaknesses
### Strengths
- Codes, messages, and levels originate from checked source catalogues.
- Duplicate names, malformed declarations, and invalid levels fail generation.
- The firmware catalogue is fixed-size and keeps messages in AVR program memory.
- Message lookup validates its index, and driver operations expose explicit error codes.

### Remaining weaknesses
- The public lookup exposes text but not severity, owner, or recovery policy.
- Driver dependencies collapse underlying causes, so callers cannot inspect an error chain.
- Catalogue size and several 8-bit consumers do not share a documented extension policy.
- Panic is a direct halt path; no structured runtime record or tested safe-state escalation exists.
