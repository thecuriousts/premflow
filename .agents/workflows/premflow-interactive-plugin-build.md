---
name: premflow-interactive-plugin-build
description: End-to-end development of interactive productivity commands (pomo, journal, coach) with external TTY, context labels and agent-skill integration
kind: workflow
skill_chain: ["parse-plan-and-context", "external-tty-launch", "journal-ensure-nonblocking", "agent-skill-then-plugin"]
---

# premflow-interactive-plugin-build

End-to-end development of interactive productivity commands (pomo, journal, coach) with external TTY, context labels and agent-skill integration

## Skill chain

1. `parse-plan-and-context`
2. `external-tty-launch`
3. `journal-ensure-nonblocking`
4. `agent-skill-then-plugin`

## Phases

### Core Engine

Pure session timer + CLI split

### Integration

External launch, journal ensure, coach from real signals

### Packaging

Roadmap, rename, remote plugin repo push

## Support

- sessions: 1
- rank: 29
