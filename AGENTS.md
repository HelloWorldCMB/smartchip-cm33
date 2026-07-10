# AGENTS.md

## Purpose

This file defines the default behavior and working principles for the AI agent within this repository.

Unless project-specific instructions override these rules, follow the guidelines below.

---

# Communication

## Language

- Default to answering in Chinese.
- Keep explanations concise, accurate and technically clear.
- If the user explicitly requests another language, follow the user's request.

## Clarification

When requirements are ambiguous:

- Ask questions instead of guessing.
- Do not invent missing requirements.
- Clearly state any assumptions.

## Scope

Only perform the task requested by the user.

Do not expand the scope without confirmation.

---

# Working Principles

## One Goal at a Time

Each interaction should focus on a single objective.

Avoid mixing unrelated refactoring, optimization, formatting and feature development into one change.

## Minimal Changes

Prefer the smallest change that correctly solves the problem.

Avoid unnecessary refactoring.

Avoid changing existing behavior unless explicitly requested.

## Reuse First

Before introducing new code:

- Look for existing implementations.
- Prefer reuse over duplication.
- Avoid unnecessary wrappers and abstractions.

---

# Before Modifying Code

Before making any modification, explain:

- Why the change is needed.
- Which files are expected to be modified.
- What this change will solve.
- What will NOT be changed.

Do not silently edit code.

---

# After Modifying Code

After completing a modification:

Summarize:

- Files modified.
- What changed.
- Why it changed.
- Any remaining issues.
- Suggested next step.

Unless the user explicitly requests otherwise, stop after completing the current task.

Do not continue implementing additional features automatically.

---

# Code Style

Follow the existing style of the repository.

Do not introduce a different coding style.

If no existing style exists:

- Keep code simple.
- Prefer readability.
- Keep functions focused.
- Avoid deep nesting.
- Keep error handling explicit.
- Avoid unnecessary abstractions.

---

# Output Format

## Code

Always use fenced code blocks.

Do not mix explanations inside code blocks.

Explain code outside the block.

## Commands

Shell commands should always be provided in standalone code blocks.

Example:

```bash
make
```

Do not prepend prompts such as:

```text
$
#
>
```

## Diff

When showing a proposed modification, prefer diff format whenever practical.

```diff
- old
+ new
```

## File Locations

Use IDE-clickable file references.

Example:

```text
src/main.c:120
```

Do not prepend numbering or extra text.

---

# Safety

Do not perform potentially destructive actions without explicit confirmation.

Examples include but are not limited to:

- deleting files
- force overwriting
- force pushing
- history rewriting
- database reset
- formatting an entire repository

Explain the impact before suggesting such operations.

---

# Build, Test and Execution

Do not automatically:

- build
- compile
- run
- deploy
- benchmark
- execute tests

unless the user explicitly requests it.

You may recommend commands, but clearly indicate they have not been executed.

---

# Dependency Management

Unless requested:

- do not introduce new dependencies
- do not replace existing libraries
- do not upgrade package versions
- do not change build systems

---

# Refactoring

Do not perform large-scale refactoring unless explicitly requested.

Avoid:

- renaming unrelated symbols
- moving files
- changing project structure
- formatting unrelated code

---

# Documentation

When modifying public APIs or behavior, update related documentation if appropriate.

Do not generate unnecessary documentation.

---

# Temporary Code

If temporary implementations are necessary:

- clearly mark them as TODO
- explain what remains unfinished
- describe the intended future implementation

Never leave placeholder code without explanation.

---

# Error Handling

Prefer explicit error handling.

Do not silently ignore failures.

Do not suppress warnings without justification.

---

# Performance

Do not optimize prematurely.

Only optimize when:

- requested by the user
- supported by profiling
- clearly necessary

Prefer correctness over micro-optimizations.

---

# Security

Never reduce security for convenience.

Avoid:

- hardcoded secrets
- disabled authentication
- disabled certificate verification
- insecure defaults

Unless explicitly requested for testing purposes.

---

# Workflow

Default workflow:

1. Understand the request.
2. Read relevant code.
3. Explain the planned change.
4. Make the minimal modification.
5. Summarize the result.
6. Stop.

---

# General Principles

- Think before changing.
- Keep changes minimal.
- Preserve existing behavior whenever possible.
- Prefer maintainability over cleverness.
- Do not surprise the user.