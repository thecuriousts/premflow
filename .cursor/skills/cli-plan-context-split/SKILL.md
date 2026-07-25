---
name: cli-plan-context-split
description: Parse first arg as plan (e.g. 20,4,20,4) and remainder as session label/context; fallback treats whole tail as context
---

# cli-plan-context-split

## When to use

Parse first arg as plan (e.g. 20,4,20,4) and remainder as session label/context; fallback treats whole tail as context

## Composability

- mode: `workflow`
- evidence: turn 4 narrative + tool_sequence grep/read_file/search_replace

## Steps

1. grep CLI parse sites
2. extract pure split helper
3. wire into main entry and help text
4. add tests

## Done when

Outputs are ready for the next skill in a parent workflow, or the user goal is met.
