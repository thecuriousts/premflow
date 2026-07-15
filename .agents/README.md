# Agent control surface (premflow)

**Source of truth for next work:** [`designs/coming-next.md`](../designs/coming-next.md)

## Quick steer

1. Run **SN-1 dogfood** before product code: `make test`, `./build/premflow`, `./build/premflow review`.
2. Pick one **SN-*** card from the roadmap; do not invent parallel architecture.
3. Load project skills under `.agents/skills/` when they match (explore-repo-readonly, mvu-refactor-plan, …).
4. Grok Build skills/plugins/hooks are harness controls — see coming-next **§15**, not C product code.

## Skills in this repo

| Skill | Role |
|-------|------|
| `explore-repo-readonly` | Map structure before CMake/MVU |
| `mvu-refactor-plan` | elomaxz MVU refactors |
| `src-tree-reorganize` | Layout under `src/` |
| `subagent-delegation` / `subagent-explore-report` | Broad readonly exploration |

## Iron-peak

Ledger + smart projection + optional external AI — plain text stays SoT. See fusion surplus in `designs/coming-next.md`.
