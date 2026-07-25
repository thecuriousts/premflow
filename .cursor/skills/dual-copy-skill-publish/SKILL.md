---
name: dual-copy-skill-publish
description: Keep git-tracked project copy under .agents/skills/ while maintaining identical copy under {REPO_ROOT} and pushing both to skills remote
---

# dual-copy-skill-publish

## When to use

Keep git-tracked project copy under .agents/skills/ while maintaining identical copy under {REPO_ROOT} and pushing both to skills remote

## Composability

- mode: `workflow`
- evidence: turns 10,19,22,23 with run_terminal_command + search_replace sequences

## Steps

1. write real tree in project
2. copy to {REPO_ROOT}
3. update symlinks
4. push to skills repo
5. verify hashes

## Done when

Outputs are ready for the next skill in a parent workflow, or the user goal is met.
