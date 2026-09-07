# Summary: premflow v2 Design Doc (ce32ca68)

**Produced:** 2026-06-03  
**Files:** /tmp/grok-design-doc-ce32ca68.md (full) + this summary

## Key Sections in Full Doc
- **Title/Metadata, Overview, Background & Motivation**: Cites exact post-phase1 state from exploration: 36KB binary, elomaxz_run_batch@main.c:39, show_review@ui.c:64 (smart curated, path copies for static hazard fix, POMO de-emph), parse_argv review_full@app.c:374, append_entry@core.c:182, list_active_tasks@core.c:280 (raw prefixes + dup header), complete_task clean@370, stats/search still system(grep), .stellarfusion/state.json debt+phase1 notes, real ~/.premflow/log.txt (dups "Second task", 40 pomos), todo.txt, journals. Pre-phase1 "POMO spam buries signal".
- **Goals/Non-Goals**: Short polish (C only), med heuristics, long v2 local LLM (summarize→journal, semantic, coach). Preserve philosophy, plain text, one-shot, <50KB core.
- **Proposed Design**: 2 mermaids (arch layers, sequence for review --ai). Short-term: clean todos, dedup, date groups, pure-C stats (remove 4 system calls), C full-review (no tail), journal --from-review, high-value heuristics ( ! + len + recency + type), config AI fields. Long: context builder (recent + opt hist redacted), editable prompts in ~/.premflow/prompts/, embeddings sidecar jsonl, drift detection. Code sketches for list_active_tasks clean, parse extensions.
- **API/Interface Changes**: Before/after CLI, PremflowMsg + review_ai/stats_pretty, parse_argv + pf_update/view updates, show_review sig change. New UX: review --ai (or "ai"), stats --pretty, journal --from-review.
- **Data Model**: None breaking. Optional sidecars only (index.jsonl, prompts/*.txt).
- **Alternatives (3 detailed)**: (1) link llama.cpp (rejected: size 5MB+, startup, C++ vs pure C11). (2) direct shell ollama/curl in ui/core (secondary: brittle parse, mixes concerns). (3) external helper (chosen: premflow-ai reads plain files, calls local ollama; zero core bloat, matches "tiny clean" + "external helper" req; + C fallback popen).
- **Security/Privacy, Observability, Rollout**: Local-only, opt-in hist+redact, no net in core; graceful; staged presence-based (not compile flags). PRs keep buildable.
- **Open Qs**: helper name, default model, POMO detail level, structured vs text, hist redaction strictness.
- **Key Decisions (7)**: external helper only for LLM; plain text forever; ollama primary; graceful+opt-in; C polish first; no elomaxz/runner change; prompts editable files.
- **PR Plan**: 6 ordered small slices (1: polish review/todos/dedup/dates; 2: pure stats/search/C-full; 3: journal+heuristics+ai-config; 4: stub+parse+graceful+docs; 5: helper contract+prefer+contrib ex; 6: semantic+sidecar+coach+v2). Each testable, cites design.

## Concrete Exploration Used
- list_dir (root, src, docs, `$HOME` + dot via terminal ls/find).
- read_file: all src/*.c/h (full or chunks: main:39 batch, app parse 327-391 + update 190 + view, effects, core paths 48/182/280/315/370, ui show_review 64-185 + stats 34 + search 187), premflow.h/app.h, README, docs/architecture.md, .stellarfusion/state.json (phase1_complete, fused concepts, debts), ~/.premflow/{log.txt (87 lines), todo.txt (13), config, journal/*}, cliconsolehelp.txt, CMakeLists, tests/test.c, elomaxz.h snippet.
- run_terminal: ls -a dots, cat data, ./build/premflow review (smart output: pending 7+...5more, wins 4, notes 5, dones dups, 40 pomos), stats (ugly), --help, binary size 36K, ollama version/list/ps (qwen2.5:7b 4.7GB, nomic), time ollama run (~4.3s), grep for llm (none in src), find no AGENTS.md root.
- web: ollama API /generate (curl json, stream:false, response), llama.cpp embed sizes/tradeoffs, CLI AI patterns (sidecar/helper common).
- Verified phase1: smart default, full via --full, path copy fix, DONE clean, help updated. Stats/search still shell as debt.

## Major Design Decisions & Tradeoffs
- **LLM arch**: external helper primary (pros: 36KB core forever, pure C, user control, plain-file readers; cons: 2-install for full feat) + direct fallback. Rejects linking (philosophy violation).
- Slices ensure "always buildable/testable/releasable" (no AI dep in make test).
- UX keeps `review` instant/C-only; --ai adds value (latency target <10s).
- Quant: 36KB, log small, ~4s LLM, heuristics for "high value".
- Risks called out (halluc, latency, privacy) + mitigations.
- References cite exact lines/paths.

## Output Verification Notes
Full doc ~ matches requested structure (adapted OpenQ/Refs; added KeyDecisions + PRPlan as required). Concrete everywhere. No new source files created (only /tmp/ outputs per task). Philosophy respected (no bloat proposals). Ready for use/execute-plan.

(Full content in grok-design-doc-ce32ca68.md; this is executive summary of what was produced.)
