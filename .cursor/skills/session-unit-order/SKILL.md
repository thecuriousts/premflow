---
name: session-unit-order
description: Prevents user-systemd / UWSM / Hyprland first-login failures caused by pulling graphical-session (or compositor) targets from Persistent=true units; requires Omarchy skill on Omarchy hosts
---

# session-unit-order

## When to use

Prevents user-systemd / UWSM / Hyprland first-login failures caused by pulling graphical-session (or compositor) targets from Persistent=true units; requires Omarchy skill on Omarchy hosts

## Composability

- mode: `workflow`
- evidence: narrative turns 5,8,9,11,17,19,23 plus tool calls creating SKILL.md and symlinks

## Steps

1. read existing unit files
2. remove Wants=graphical-session.target from lingering units
3. add regression test
4. document Omarchy conditional

## Done when

Outputs are ready for the next skill in a parent workflow, or the user goal is met.
