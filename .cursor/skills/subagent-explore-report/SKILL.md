---
name: subagent-explore-report
description: Run readonly subagent exploration of premflow or elomaxz and return a structured report. Use when delegating explore/map before CMake or MVU work.
---

# Subagent explore report

## When to use

- Parent session delegates "Explore premflow / elomaxz at {REPO_ROOT}"
- Subagent turn pattern: **UpdateCurrentStep → Glob → Grep → Read** (corpus subagent turns)
- Need structure, CMake/Makefile, MVU entry points before parent implements

## Steps

1. **Glob** key paths (`CMakeLists.txt`, `main.c`, `include/`, `src/`).
2. **Grep** MVU/model symbols (`model_`, `Program`, `elomaxz`).
3. **Read** README, build files, and primary entry (`main.c` / `app.c`).
4. Return concise report sections:
   - Directory layout
   - Build system (CMake vs Makefile)
   - MVU/state entry points
   - Dependencies (e.g. elomaxz FetchContent)
   - Risks or gaps for the parent's plan

## Output format

Use numbered sections; no full file dumps. End with **Return:** one paragraph for the parent agent.

## Constraints

Readonly — no refactors in explore subagents unless user explicitly asks to implement.
