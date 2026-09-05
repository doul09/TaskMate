# TaskMate RTOS ![](doc/icon_64.png)

**Microcontroller Unit - Real-Time Operating System**

---

## ▶️ Introduction

**TaskMate is a personal project focused on learning C programming and real-time operating system
design through hands-on practice, experimentation, and iterative development.**

At a much smaller scale and within my own limits, this project is also a way to retrace—step by
step—the kind of questions and discoveries that shaped early systems like Unix, by exploring what
the fundamental primitives of an operating system should be and how they can be implemented from scratch.

The TaskMate operating system is designed for microcontroller. Its architecture is structured to
maintain a clear separation between build
logic, system behaviour, and hardware dependencies, ensuring both portability and maintainability.

> <span style="color:green"> **Project Stats (v0.28 [^1] )**</span>
>
>  <span style="color:green">725 commits • 150 source files • 9247 lines of code •
> binary size: 13610 bytes (Flash) • RAM usage: 1779 bytes</span>

> ⚠️ <span style="color:red">**Development Status**</span>
>
> <span style="color:red">TaskMate is currently in active development and
> should be considered **experimental**. While the core system and architecture are
> functional, many components are still evolving. It is **not yet suitable for production use**,
> and both API and internal structures may change without notice.</span>


---

## ⬆️ TaskMate Layers - Run Time

![System Layer Diagram](doc/TaskMate_layers_v10.png)


The diagram shows the current layered architecture of TaskMate.
Each layer communicates primarily with its direct neighbours, following a strict top-down model to
maintain clear boundaries and avoid hidden dependencies.

Some components, such as interfaces and the lightweight libc, play a transversal role across multiple
layers. Rather than breaking the architecture, they provide controlled and well-defined access points
that help decouple the system while preserving its structure.

System features such as messaging, timing, I/O, and services remain fully accessible to user
tasks—but always through controlled and indirect interactions.

---

## ⚙️ Build system

TaskMate uses a custom **build system** that fully manages dependencies and workflow.

- Automatic recompilation based on file changes, including headers and configuration files.
- CLI commands like `make upload`, `make push` and `make backup`.

**Portability relies mostly on build-time source selection, with minimal use of preprocessor logic.**

![Build system](doc/build_v3.png)

---

## ⏱️ Real-Time Behaviour

Although TaskMate includes pre-emptive scheduling and a software real-time clock,
it **is not yet a true real-time operating system** in the strict sense.

At its current stage, TaskMate guarantees **task switching** and **time slicing** with good stability,
but it does not yet ensure **hard real-time determinism.**

System latency and jitter are acceptable for testing and lightweight applications,
yet they remain **non-deterministic** under specific conditions such as nested interrupts,
driver contention, or prolonged critical sections.

---

## 🧩 Key Concepts

> A few core ideas shape the design of TaskMate and guide its evolution:


**HAL (Hardware Abstraction Layer)**

The HAL isolates all hardware-specific details behind a consistent interface.
It allows the system to remain portable and predictable, regardless of the underlying architecture, MCU, or board.

**SysCall (System Call Layer)**

The system call layer acts as a controlled boundary between user space and the system core.
It ensures that all interactions with the kernel are explicit, validated, and well-defined.


**Interfaces**

Interfaces provide neutral contracts between layers.
They help reduce coupling by defining shared types and behaviours without exposing implementation details,
making the system easier to evolve and refactor.


**AutoCode**

AutoCode is used to generate parts of the system from simple configuration files.
It helps maintain consistency, reduce boilerplate, and keep the overall structure aligned with the intended architecture.
All generation happens at build time, with static allocation, ensuring zero runtime overhead and fully deterministic behaviour.

---

## 🧭 About ChatGPT and TaskMate

Although **no code from ChatGPT is ever copied directly** into the TaskMate source
tree, the project would never have reached its current level of maturity without
the assistance of AI. ChatGPT has been a great tool for structuring ideas,
learning new concepts, and refining both code and architectural design. It
provides technical guidance.

Codex is now also used, mainly for audits and complex refactors. It runs in a
sandboxed environment on a separate machine and under a dedicated account.
Its changes go through a double merge before reaching the main project branch;
this review process does not make AI output automatically correct.

---

## ️📜 License

This software is distributed under the **BSD-2-Clause License**.

You may use, modify, and redistribute it in source or binary form,
provided that you keep the copyright notice, license conditions,
and disclaimer as described in the `LICENSE` file.

---

## 📟 Hardware setup: AVR8 - ATmega2560 - Arduino Mega board

![Arduino mega board](doc/hardware_mega.jpg)

---

## 📑 Documentation & books 📚

- **Project development status** — see [Project Progress](doc/progress.md)
- **Changelog** — version history: see [CHANGELOG](./CHANGELOG)
- **C Style Guide** — best practices (pointers, errors, etc.): see [code best practices](./doc/C_code_best_practices.md)

**Architecture :**

You will find more information about the architecture in the files under `doc/architecture/`.
These files contain information about the development history, current implementation,
strengths and weaknesses of the source code.

**Books :**

- La référence du C norme ANSI-ISO, author Claude Delannoy, publisher Eyrolles. ISBN 2-212-09036-6
- Microcontrôleurs AVR : des ATtiny aux ATmega, author Christian Tavernier, publisher Dunod. ISBN 978-2-10-074417-6
- The markdown guide, author Matt Cone, publisher Amazon. ISBN 9798656504492
- Hands-On RTOS with Microcontrollers, author Brian Amos, publisher Packt. ISBN 978-1-83882-673-4
- Making Embedded Systems, author Elecia White, publisher O'Reilly. ISBN 978-1-098-15154-6
- Operating System Design, The Xinu Approach, third edition, author Douglas Comer, publisher CRC Press, ISBN 978-1-032-98099-7
- The design and implementation of the FreeBSD operating system, second edition, authors Marshall Kirk McKusick, George V. Neville-Niel and Robert N.M. Watson, publisher Addison-Wesley, ISBN 978-0-312-96897-2

[^1]: ⚠️ Warning: Versions 0.71, 1.37, 2.71, 3.14, 4.2 and 5.11 are intentionally skipped. Universe backward compatibility constraints apply.
