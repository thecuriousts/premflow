# Agent control surface (premflow)

**Source of truth:** [`designs/coming-next.md`](../designs/coming-next.md)

## Two jobs for Grok

| Job | Goal | Entry |
|-----|------|--------|
| **Life flow** | Grok *uses* premflow (review, pomo, capture) as tools — preferred AI path vs local ollama | coming-next **§8**, **§15**, **SN-4** |
| **Code flow** | Change the C repo against SN cards | SN-1…SN-6 + skills below |

## Quick steer

1. **SN-1 dogfood:** `make test`, `./build/premflow`, `./build/premflow review`.
2. Life coach: run real `premflow` commands; prefer CLI writes over inventing log lines.
3. Code work: one **SN-*** card at a time; Done when + Verify from the roadmap.
4. Harness knobs: skills, plugins (`/plugins`, `grok plugin …`), hooks — see §15.

## Skills in this repo (coding)

| Skill | Role |
|-------|------|
| `explore-repo-readonly` | Map structure before CMake/MVU |
| `mvu-refactor-plan` | elomaxz MVU refactors |
| `src-tree-reorganize` | Layout under `src/` |
| `subagent-delegation` / `subagent-explore-report` | Broad readonly exploration |

## Iron-peak

Ledger (premflow) + projection (C review) + judgment (Grok plugin/skills). Plain text stays SoT.
