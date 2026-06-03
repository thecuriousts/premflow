---
name: mvu-refactor-plan
description: Plan and implement CMake + elomaxz MVU refactors in premflow. Use when user references elomaxz, MVU, CMake refactor plan, or attached implementation plan.
---

# MVU refactor (premflow)

## When to use

- User attaches or references an implementation plan for CMake + elomaxz MVU
- Refactoring premflow model/view/update structure
- Integrating patterns from elomaxz repo

## Steps

1. Read the attached plan and existing `CMakeLists.txt`, model, and app entry points.
2. Grep for symbols mentioned in the plan before moving files.
3. Implement in small steps: CMake → headers → `.c` → build after each logical chunk.
4. Run build; fix compiler errors before next step.
5. Update README/license only after core build passes.

## Constraints

- Follow elomaxz naming and MVU boundaries from the plan — do not invent parallel architecture.
- Prefer minimal diffs; avoid drive-by reformat of unrelated C files.
- If ColumnLimit breaks signatures, fix layout in context — do not truncate identifiers.

## Done when

Build succeeds and plan checklist items are addressed or explicitly deferred with reason.
