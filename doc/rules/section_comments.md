# TaskMate — Section Comment Formatting Rules

## Purpose

This document defines the standard formatting and usage rules for section comments
in TaskMate C source (`.c`) and header (`.h`) files.

The goal is to improve file readability and navigation without adding redundant
comments or changing code behavior.

---

## General principles

Section comments describe the structure of a file, not the behavior of individual
functions or variables.

Do not add comments that merely repeat information already made clear by explicit
function, variable, type, or macro names.

Use section comments to make the organization of a file immediately visible when
scrolling through the source.

Do not modify program behavior while applying these formatting rules.

Do not rename symbols, reorder code with semantic impact, or perform unrelated
refactoring unless explicitly requested.

Do not create empty sections.

Keep existing meaningful comments unless they conflict with these rules or are
clearly redundant.

---

## Section hierarchy

TaskMate uses three visual levels.

### Level 1 — Major file section

Use `=` separators for major structural parts of a file.

```c
/* =============================================================================
 * Declarations
 * ===========================================================================*/
```

Typical major sections include:

- `Declarations`
- `Implementation`
- `Public API`

Major sections should be used sparingly.

---

### Level 2 — Functional group

Use `-` separators for groups inside a major section.

```c
/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/
```

Typical groups include:

- `Constants`
- `Private types`
- `Private variables`
- `Private function prototypes`
- `Macros`
- `Types`
- `Driver lifecycle`
- `I2C drivers`
- `RTC operations`
- `USART operations`
- `Error handling`

Names should describe the role of the following code, not merely its syntax.

---

### Level 3 — Minor subgroup

For small subdivisions inside a functional group, use a simple comment.

```c
/* 
 * Scan operations 
 */
```

Use this level only when it materially improves readability.

Do not use decorative separators for minor subgroups.

---

# C source files

## Recommended structure

A `.c` file should normally follow this general organization when applicable:

```c
/*
 * TaskMate Project
 * ...
 */

/**
 * @file example.c
 * @brief ...
 */

/* ============================================================================
 * Declarations - Include
 * ========================================================================== */

#include "example.h"

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

/* -----------------------------------------------
 * Private types
 * ---------------------------------------------*/

/* -----------------------------------------------
 * Private variables
 * ---------------------------------------------*/

/* -----------------------------------------------
 * Private function prototypes
 * ---------------------------------------------*/

/* ============================================================================
 * Implementation - Functions
 * ========================================================================== */

/* -----------------------------------------------
 * Functional group 1
 * ---------------------------------------------*/
```

Only sections containing actual content should be present.

---

# Header files

Header files use the same visual hierarchy but should reflect the structure of a
public or internal interface rather than copy the `.c` layout directly.

## Recommended structure

```c
/*
 * TaskMate Project
 * ...
 */

/**
 * @file example.h
 * @brief ...
 */

#ifndef EXAMPLE_H
#define EXAMPLE_H

/* ============================================================================
 * Includes
 * ========================================================================== */

#include ...

/* ============================================================================
 * Public definitions
 * ========================================================================== */

/* -----------------------------------------------
 * Constants
 * ---------------------------------------------*/

/* -----------------------------------------------
 * Types
 * ----------------------------------------------*/

/* ------------------------------------------------
 * Macros
 * ----------------------------------------------*/

/* ============================================================================
 * Public API
 * ========================================================================== */

void anyFunction(void);
...

#endif
```

Again, omit empty sections.

---

# What not to do

## Do not comment every function

Avoid comments such as:

```c
/* Initialize driver */
static bool driverInit(...);
```

when the function name already makes the purpose obvious.

Add a function-specific comment only when it provides information not expressed
by the function name or signature, such as:

- non-obvious preconditions;
- ownership rules;
- hardware constraints;
- concurrency requirements;
- side effects;
- unusual error semantics.

---

## Do not create decorative noise

Avoid excessive layers such as:

```c
/* ============================================================================
 * Drivers
 * ========================================================================== */

/* ----------------------------------------------------------------------------
 * I2C
 * -------------------------------------------------------------------------- */

/* -------------------------
 * Scan
 * ----------------------- */
```

Use a simple minor comment instead:

```c
/* 
 * Scan operations 
 */
```

---

## Do not change semantics during formatting

When applying these rules, do not:

- alter control flow;
- change declarations;
- rename functions or variables;
- change qualifiers;
- reorder code when ordering may matter;
- change includes unless required by a separate task;
- modify generated code unless explicitly requested;
- perform unrelated cleanup.

Section formatting should normally produce comment-only changes.

Whitespace changes directly required by the section formatting are acceptable.

---

# Codex application rules

When asked to apply this document to existing TaskMate files:

1. Read the complete target file before modifying it.
2. Identify the existing logical structure.
3. Add or normalize section comments according to this document.
4. Preserve useful existing documentation comments.
5. Remove only structural comments made redundant by the new organization.
6. Do not create empty sections.
7. Group implementation functions according to logical responsibility.
8. Avoid moving functions unless necessary and safe.
9. Prefer comment-only changes.
10. Do not alter code behavior.
11. Do not perform unrelated style cleanup.
12. Keep the resulting diff easy to review.
13. If the correct logical grouping is ambiguous, ask user.

The purpose of this operation is readability and source navigation, not code
refactoring.
