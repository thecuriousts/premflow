---
name: src-tree-reorganize
description: Reorganize scattered premflow sources into src/ with CMake updates. Use when user asks to tidy layout, move files to src, or clean root-level clutter.
---

# Src tree reorganize

## When to use

- User wants sources moved from repo root into `src/` (or similar tidy-up)
- "make the project neat", scattered `.c` / `.h` at top level
- After MVU refactor when file layout is still messy

## Steps

1. **Glob** all `*.c`, `*.h`, and `CMakeLists.txt` under `{REPO_ROOT}`.
2. **Grep** includes and `add_executable` / `target_sources` references.
3. Propose target layout (e.g. `src/`, `include/`) — minimal moves per step.
4. **Write** / **StrReplace** paths in CMake and `#include` lines together.
5. **Shell** `cmake --build` (or project build command) after each batch of moves.
6. Summarize what moved and what still references old paths.

## Constraints

- Do not edit the user's attached plan file if one exists.
- One logical batch per user "proceed" — build between batches.

## Done when

Tree matches agreed layout and build passes.
