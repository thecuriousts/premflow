---
name: external-tty-launch
description: Spawn interactive tools like pomo in external terminal (Ghostty/Kitty) via D-Bus or preferred app to keep agent chat non-blocking
---

# external-tty-launch

## When to use

Spawn interactive tools like pomo in external terminal (Ghostty/Kitty) via D-Bus or preferred app to keep agent chat non-blocking

## Composability

- mode: `workflow`
- evidence: turn 16-17 tool calls and narrative on pf-focus + ghostty/kitty handling

## Steps

1. detect available terminal
2. prefer tab over window when possible
3. pass plan+context args
4. return control immediately

## Done when

Outputs are ready for the next skill in a parent workflow, or the user goal is met.
