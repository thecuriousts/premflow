# premflow

![premflow](/project-intro-picture-grok-image.png)

**Tiny. Clean. Powerful.**  
A minimalist productivity CLI tool written in pure C — zero bloat, maximum daily time saved.

---

## For Users

### Features

- 📝 Quick notes & wins logging
- ✅ Task management (add, list, complete)
- 🍅 Pomodoro timer with sound notifications
- 📖 Daily journal with beautiful template
- 🔍 Search across logs & tasks
- 📊 Personal stats dashboard
- ⚙️ Customizable sounds via config

### Installation

```bash
git clone https://github.com/thecuriousts/premflow.git
cd premflow

./build.sh 
```

####  User-local Install (No sudo)

```bash
make install
```

> Make sure `~/.local/bin` is in your `PATH`.

#### Run without installing

```bash
./premflow
```

### Usage

#### Basic Commands

```bash
premflow                    # Show help
premflow note "Great idea!" # Log a quick note
premflow win "Nailed the demo"
premflow task add "Buy milk"
premflow task list
premflow task done 2
premflow pomo 25            # Start 25-min pomodoro
premflow journal            # Open today's journal (creates template if new)
premflow stats              # Show lifetime stats
premflow review             # Daily review (recent wins + tasks)
premflow search "meeting"   # Search logs & tasks
premflow edit todo          # Edit tasks in $EDITOR (default: nano)
premflow config sound       # Customize sound notifications
```

#### Example Daily Workflow

```bash
# Morning
premflow journal
premflow task add "Finish project proposal"
premflow task add "Review pull requests"

# Deep work
premflow pomo 50

# End of day
premflow win "Shipped v2.0"
premflow review
```

### Configuration

Edit sound settings:

```bash
premflow config sound
```

#### Example config (`~/.premflow/config.txt`):

Find out your linux distros `PLAYER` and Sounds.
Update `POMO_START`, `POMO_COMPLETE`, `TASK_COMPLETE`

```ini
PLAYER=paplay
POMO_START=paplay /usr/share/sounds/freedesktop/stereo/phone-incoming-call.oga >/dev/null 2>&1
POMO_COMPLETE=paplay /usr/share/sounds/freedesktop/stereo/complete.oga >/dev/null 2>&1
TASK_COMPLETE=paplay /usr/share/sounds/freedesktop/stereo/bell.oga >/dev/null 2>&1
```

Empty value = disable that sound.

---


### Useful Make Targets

```bash
make              # Build the binary
make test         # Run all 8 comprehensive tests (file I/O mocking)
make clean        # Remove build artifacts
make install      # Install system-wide (/usr/local)
make install PREFIX=~/.local   # User-local install (no sudo)
make uninstall    # Remove installed files
```

### Testing

The test suite includes:

- String trimming logic
- Path generation
- Append / complete task with real temp files
- Config template creation
- Journal path & creation
- Pomodoro edge cases (0, negative values)
- Full journal template verification

All tests use `mkstemp()` for safe, isolated file I/O testing.


### Project Structure

```
premflow/
├── premflow.h
├── main.c          # CLI entry point
├── core.c          # Business logic + error handling
├── ui.c            # Display / output functions
├── test.c          # Comprehensive test suite
├── Makefile
├── README.md
```

### Screenshots

![build](/screenshots/build.sh-2026-04-24_22-30-24.png)
![make](/screenshots/make-install-2026-04-24_22-32-44.png)
![usecase](/screenshots/usecase-2026-04-24_20-09-26.png)

### Philosophy

- One tiny binary (~26KB)
- Zero external dependencies (standard C + common Unix tools)
- Built for speed and daily personal use
- Clean separation: Logic vs Display
- Proper error handling and exit codes

### Todos

-  AUR packaging for Arch Linux


---

**premflow** — because your tools should get out of the way.

---

Made with ❤️ for focused, productive humans.