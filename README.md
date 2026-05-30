# premflow

![premflow](/project-intro-picture-grok-image.png)

**Tiny. Clean. Powerful.**  
A minimalist productivity CLI tool written in pure C — zero bloat, maximum daily time saved.

### Why you need to use premflow

> Because your tools should get out of the way — not throw a party. premflow 
> delivers a small binary with minimal dependencies, zero bloat, and zero 
> reasons left to procrastinate. Tiny. Clean. Powerful. Like your morning 
> coffee, but with better error handling. It’s the CLI that respects your time 
> so much it refuses to waste any of its own — and honestly, installing a 
> 200MB Electron app just to write “buy milk” is a crime against humanity.
> Your excuses have nowhere to hide.

---

### Features


- 📝 Quick notes & wins logging
- ✅ Task management (add, list, complete)
- 🍅 Pomodoro timer with sound notifications
- 📖 Daily journal with beautiful template
- 🔍 Search across logs & tasks
- 📊 Personal stats dashboard
- ⚙️ Customizable sounds via config

![usecase polished](/usecase.png)


### Installation

Requires **CMake 3.14+** and a C11 compiler. The [elomaxz](https://github.com/p10ns11y/elomaxz) MVU library is fetched automatically via CMake `FetchContent`.

```bash
git clone https://github.com/thecuriousts/premflow.git
cd premflow

./build.sh

# Run without installing
./build/premflow
```

**Offline / local elomaxz** (skip network fetch):

```bash
export ELOMAXZ_SOURCE_DIR=/path/to/elomaxz
./build.sh
# or: cmake -B build -DELOMAXZ_SOURCE_DIR=/path/to/elomaxz && cmake --build build
```

#### User-local Install (No sudo)

> Make sure `~/.local/bin` is in your `PATH`.

```bash
make install   # installs to ~/.local/bin (default)
```

The [Makefile](/Makefile) wraps CMake and sets `PREFIX ?= $(HOME)/.local` by default.

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

Find out your Linux distro's `PLAYER` and sounds.
Update `POMO_START`, `POMO_COMPLETE`, `TASK_COMPLETE`.

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
make              # Configure and build (output in build/)
make test         # Run all 8 comprehensive tests via ctest
make format       # Apply clang-format to all sources (.clang-format)
make format-check # Fail if sources are not formatted (used in CI)
make clean        # Remove build/ directory
make install      # Install to ~/.local/bin (recommended)
make install PREFIX=/usr/local   # System-wide (requires sudo)
make uninstall    # Remove installed binary
```

Formatting uses [clang-format](https://clang.llvm.org/docs/ClangFormat.html) with the repo `.clang-format` config (requires `clang` on PATH).

Or with CMake directly:

```bash
cmake -B build && cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix ~/.local
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


### Philosophy

- One small binary
- [elomaxz](https://github.com/p10ns11y/elomaxz) MVU core (functional update + imperative effects) + standard C + common Unix tools
- Built for speed and daily personal use
- Clean separation: update/view (app) vs effects (I/O) vs display helpers (ui)
- Proper error handling and exit codes


### Project Structure

```
premflow/
├── premflow.h      # Shared types and core/ui API
├── main.c          # Bootstrap + argv → message + elomaxz_run_batch
├── app.c / app.h   # Model, messages, init/update/view
├── effects.c       # handle_cmd — file I/O, editor, pomodoro
├── core.c          # Business logic + error handling
├── ui.c            # Display / output functions
├── test.c          # Comprehensive test suite
├── CMakeLists.txt  # FetchContent(elomaxz) + targets
├── Makefile        # Thin CMake wrapper
├── README.md
```

### Screenshots

![build](/screenshots/build.sh-2026-04-24_22-30-24.png)
![make](/screenshots/make-install-2026-04-24_22-32-44.png)
![usecase](/screenshots/usecase-2026-04-24_23-23-00.png)


### Todos

- AUR packaging for Arch Linux

---

**premflow** — because your tools should get out of the way.

---

Made with ❤️ for focused, productive humans.  
