# 🧭 Code and file prefixes — One-page guidelines

## 🏷️ In TaskMate, prefixes are used deliberately and sparingly.

- Each prefix corresponds to a **clear conceptual responsibility**
- No prefix overlaps with another
- No prefix is decorative or redundant

---

## 📋 Below is a concise overview of the existing prefixes and their intent.

### ● `tm_` — Project identity

**Everything that is specific to TaskMate itself**

- Acts as a global namespace
- Protects against external symbol collisions
- Serves as a mental anchor: “this code belongs to the TaskMate core”

👉 To be used **sparingly** and only when the project identity matters.

---

### ● `DRV_` — Driver interface constants

**Portable control operations and state definitions shared by drivers**

- Names the constants declared by the neutral driver interface
- The corresponding generic interface headers use the `drv_<driver>.h` filename form
- Keeps the driver control protocol distinct from TaskMate core constants
- Uses `DRV_CTRL_` for every operation accepted by a driver control function

👉 This prefix is reserved for the **portable driver contract**, not for
hardware-specific implementation details.

---

### ● `sc_` — Security boundary (system calls)

**Controlled and limited access from tasks to system functionality**

- Exposes a restricted API to user tasks
- Clearly symbolises a privilege boundary

At a glance, it reads as:

> “This code is not the kernel, but it is talking to it.”

---

### ● `gpio_` — Logical-to-physical middleware

**An intermediate semantic layer between software logic and hardware signals**

- Not HAL
- Not application code
- Not a pure driver

👉 Using a dedicated prefix explicitly acknowledges the existence of an
**intermediate semantic layer**, rather than hiding it inside unrelated components.

---

### ● `err_` — Global error system

**Cross-cutting error handling infrastructure**

- Transversal by nature
- Easy to grep
- Ideal for auditing, logging, and diagnostics

Most importantly:

👉 A transversal concern **justifies** a transversal prefix.

---

### ● `mod_` — Module database

**System-wide description of threads and drivers**

- Provides a global view of the system
- Acts as a single source of truth
- Implements a quasi *“kernel metadata”* logic

Using a dedicated prefix avoids ambiguity:

👉 These symbols **describe the system as a whole**, not just one of its parts.

---

### ● `rl_` — Run levels

**System life cycle management**

- Small and well-defined domain
- Strong conceptual meaning
- Explicit life cycle logic

👉 A minimal prefix for a **fundamental OS primitive**.

---

## Summary and why this matters

A prefix is not decoration.

> **A prefix replaces a comment — it must not become noise.**

TaskMate prefixes provide:

- A clear taxonomy
- A short and controlled list
- Strong semantic meaning
- Zero over-engineering
- Real room for future evolution

▶️ **Prefixes name conceptual boundaries**
