---
name: issue-to-preventive-skill
description: Convert a systemd race bug report into a regression doc plus composable skill, then publish dual copies
kind: workflow
skill_chain: ["session-unit-order", "dual-copy-skill-publish"]
---

# issue-to-preventive-skill

Convert a systemd race bug report into a regression doc plus composable skill, then publish dual copies

## Skill chain

1. `session-unit-order`
2. `dual-copy-skill-publish`

## Phases

### Diagnose

Audit units and confirm Linger + graphical-session race

### Fix

Edit unit and verify boot

### Extract

Write REGRESSION.md and SKILL.md

### Publish

Dual-copy to project + skills repo with Omarchy note

## Support

- sessions: 1
- rank: 29
