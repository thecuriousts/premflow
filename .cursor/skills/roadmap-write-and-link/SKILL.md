---
name: roadmap-write-and-link
description: Read goal + existing roadmap skill, scout repo, emit designs/coming-next.md and .agents/README.md, then link from README
---

# roadmap-write-and-link

## When to use

Read goal + existing roadmap skill, scout repo, emit designs/coming-next.md and .agents/README.md, then link from README

## Composability

- mode: `workflow`
- evidence: turn 5 and turn 7 tool sequences

## Steps

1. read goal and stellar-roadmap skill
2. list_dir + grep for surfaces
3. write roadmap sections (UX, LLM path, Grok Build control)
4. search_replace README and update_goal

## Done when

Outputs are ready for the next skill in a parent workflow, or the user goal is met.
