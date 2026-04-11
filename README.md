# premflow

![premflow](/project-into-picture-grok-image.png)

Small C CLI for notes, tasks, pomodoro, a daily journal, quick search, and a simple stats view. Everything lives under `~/.premflow/`:

- `log.txt` — timestamped notes, wins, pomodoros, and completed tasks
- `todo.txt` — active tasks
- `journal/journal-YYYY-MM-DD.txt` — one file per day (created with a short template the first time you open it)

## Build

```bash
gcc main.c -o premflow
```

Install the binary on your `PATH` if you like.

## Usage

Run `premflow` with no arguments for the full command list.

| Command | What it does |
|--------|----------------|
| `note "…"` | Append a note to the log |
| `task add "…"` / `task list` / `task done N` | Todos; `done` moves the line to the log as `[DONE]` |
| `win "…"` | Log a win or gratitude-style entry (`[WIN]`) |
| `journal` | Open today’s journal in `$EDITOR` (template on first open) |
| `pomo [minutes]` | Focus timer (default 25) |
| `edit` / `edit todo` | Edit log or todo file in `$EDITOR` (default `nano`) |
| `search "keyword"` | Grep log and todo files (case-insensitive, colored when supported) |
| `stats` | Counts for pomos, done tasks, notes, wins; today’s journal hint |
| `review` | Last lines of the log plus active tasks |

## Requirements

POSIX-style OS (Linux/macOS), a C compiler, `grep` (for `search`), and `tail` (for `review`). `stats` uses `date` and `ls` via the shell.
