# premflow

Small command-line helper for quick notes, todos, pomodoro timers, and a daily review. Data is stored under `~/.premflow/` (`log.txt`, `todo.txt`).

## Build

```bash
gcc -o premflow premflow_source.c
```

Optionally install the binary somewhere on your `PATH`.

## Usage

Run `premflow` with no arguments to see all commands:

- **note** — append a timestamped note to the log
- **task add / list / done** — manage a simple todo list; completing a task moves it to the log as done
- **pomo** — focus timer (default 25 minutes)
- **edit** / **edit todo** — open the log or todo file in `$EDITOR` (falls back to `nano`)
- **review** — show recent log lines and active tasks

## Requirements

POSIX-ish environment (Linux/macOS), C compiler, and `tail` for `review`.
