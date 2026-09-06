# 🚧 Architecture boundary enforcement

This document describes the build mechanism. The rules that the mechanism
enforces remain in `doc/rules/interfaces.md` and the architecture guide.

TaskMate enforces architectural boundaries **at build time** to prevent accidental
or unauthorised access to system-critical interfaces.
This mechanism relies entirely on **static checks and conventions**,
introducing **zero runtime overhead** and no impact on the final firmware size.

Boundary enforcement is handled directly by the build system and operates at three complementary levels:


## 1. Build-time validation


The Makefile validates which source files are allowed to include system-critical
headers, based on explicit allowlist rules. Any unauthorised inclusion immediately
stops the build, ensuring architectural violations are detected **before compilation.**


## 2. Compile-time guards

System-critical headers contain defensive compile-time checks. Only explicitly
authorised modules receive the required preprocessor definitions, preventing both
direct and indirect inclusion of privileged interfaces.


## 3. Structural conventions

Clear directory separation and consistent naming conventions reinforce architectural
discipline, making incorrect usage unlikely and easy to detect.


## Software-only boundary

This mechanism enforces **logical and structural separation** only. It does **not** provide
memory isolation, privilege rings, or hardware-backed protection (MMU/MPU).
Its purpose is to maintain architectural clarity, robustness, and long-term
maintainability within the constraints of embedded systems, while remaining
lightweight and fully deterministic.
