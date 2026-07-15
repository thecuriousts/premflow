#include "premflow.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

SoundConfig sounds = {0};

// ==================================================================
// String Utilities
// ==================================================================

char *ltrim(
    char *s
) {
    while (*s == ' ' || *s == '\t' || *s == '\n') {
        s++;
    }
    return s;
}

void rtrim(
    char *s
) {
    if (!s || !*s) {
        return;
    }
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end-- = '\0';
    }
}

char *trim(
    char *s
) {
    char *start = ltrim(s);
    rtrim(start);
    return start;
}

// ==================================================================
// Paths
// ==================================================================

char *data_path(
    const char *filename
) {
    static char path[512];
    char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        return NULL;
    }
    snprintf(path, sizeof(path), "%s/%s/%s", home, DATA_DIR, filename);
    return path;
}

char *journal_path(
    void
) {
    static char path[512];
    char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: HOME environment variable not set\n");
        return NULL;
    }

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char date[32];
    strftime(date, sizeof(date), "%Y-%m-%d", tm);

    snprintf(path, sizeof(path), "%s/%s/journal/journal-%s.txt", home, DATA_DIR, date);
    return path;
}

// ==================================================================
// Config
// ==================================================================

void create_config_template(
    const char *path
) {
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Error: Could not create config file at %s\n", path);
        return;
    }
    fprintf(
        f,
        "# premflow Sound Configuration\n"
        "# Leave empty to disable\n\n"
        "PLAYER=paplay\n"
        "POMO_START=paplay "
        "/usr/share/sounds/freedesktop/stereo/phone-incoming-call.oga >/dev/null 2>&1\n"
        "POMO_COMPLETE=paplay /usr/share/sounds/freedesktop/stereo/complete.oga "
        ">/dev/null 2>&1\n"
        "TASK_COMPLETE=paplay /usr/share/sounds/freedesktop/stereo/bell.oga >/dev/null "
        "2>&1\n"
    );
    fclose(f);
}

void read_config(
    void
) {
    char *path = data_path(CONFIG_FILE);
    if (!path) {
        return;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        create_config_template(path);
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        char *eq = strchr(line, '=');
        if (!eq) {
            continue;
        }

        *eq = '\0';
        char *key = trim(line);
        char *val = trim(eq + 1);

        if (!*key) {
            continue;
        }

        if (strcmp(key, "PLAYER") == 0) {
            strncpy(sounds.player, val, sizeof(sounds.player) - 1);
        } else if (strcmp(key, "POMO_START") == 0) {
            strncpy(sounds.pomo_start, val, sizeof(sounds.pomo_start) - 1);
        } else if (strcmp(key, "POMO_COMPLETE") == 0) {
            strncpy(sounds.pomo_complete, val, sizeof(sounds.pomo_complete) - 1);
        } else if (strcmp(key, "TASK_COMPLETE") == 0) {
            strncpy(sounds.task_complete, val, sizeof(sounds.task_complete) - 1);
        }
    }
    fclose(f);
}

// ==================================================================
// Core Logic with better error handling
// ==================================================================

bool ensure_dirs(
    void
) {
    char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Error: HOME not set\n");
        return false;
    }

    char dir[512];
    snprintf(dir, sizeof(dir), "%s/%s", home, DATA_DIR);

    if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Could not create directory %s\n", dir);
        return false;
    }

    char journal_dir[1024];
    snprintf(journal_dir, sizeof(journal_dir), "%s/journal", dir);
    if (mkdir(journal_dir, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error: Could not create journal directory\n");
        return false;
    }
    return true;
}

bool ledger_sanitize_body(
    const char *in,
    char *out,
    size_t out_sz
) {
    if (!out || out_sz == 0) {
        return false;
    }
    out[0] = '\0';
    if (!in) {
        return false;
    }

    size_t j = 0;
    int prev_space = 1; /* trim leading space */
    for (const char *p = in; *p && j + 1 < out_sz; p++) {
        unsigned char c = (unsigned char) *p;
        if (c == '\n' || c == '\r' || c == '\t') {
            c = ' ';
        }
        if (c == ' ') {
            if (prev_space) {
                continue;
            }
            prev_space = 1;
            out[j++] = ' ';
            continue;
        }
        if (c < 32) {
            continue; /* drop other controls */
        }
        prev_space = 0;
        out[j++] = (char) c;
    }
    /* trim trailing space */
    while (j > 0 && out[j - 1] == ' ') {
        j--;
    }
    out[j] = '\0';
    return j > 0;
}

void ledger_clean_done_body(
    const char *task_line,
    char *out,
    size_t out_sz
) {
    if (!out || out_sz == 0) {
        return;
    }
    out[0] = '\0';
    if (!task_line) {
        return;
    }

    char buf[MAX_LINE];
    snprintf(buf, sizeof(buf), "%s", task_line);
    char *clean = trim(buf);

    /* Prefer last [TODO] body (handles nested historical shapes) */
    char *todo_tag = NULL;
    for (char *p = clean; (p = strstr(p, "[TODO]")) != NULL; p += 6) {
        todo_tag = p;
    }
    if (todo_tag) {
        clean = trim(todo_tag + 6);
    } else {
        /* strip [ts] [TYPE] if present: after second ']' */
        char *p = strchr(clean, ']');
        if (p) {
            p = strchr(p + 1, ']');
        }
        if (p) {
            clean = trim(p + 1);
        }
    }

    /* Drop accidental leading type tags left in body */
    while (clean[0] == '[') {
        char *end = strchr(clean, ']');
        if (!end) {
            break;
        }
        clean = trim(end + 1);
    }

    ledger_sanitize_body(clean, out, out_sz);
}

bool ledger_line_matches_contract(
    const char *line
) {
    if (!line || line[0] != '[') {
        return false;
    }
    /* [YYYY-MM-DD HH:MM] */
    if (strlen(line) < 19) {
        return false;
    }
    if (line[5] != '-' || line[8] != '-' || line[11] != ' ' || line[14] != ':' ||
        line[17] != ']') {
        return false;
    }
    for (int i = 1; i <= 4; i++) {
        if (line[i] < '0' || line[i] > '9') {
            return false;
        }
    }
    if (line[18] != ' ' || line[19] != '[') {
        return false;
    }
    const char *type = line + 20;
    if (*type < 'A' || *type > 'Z') {
        return false;
    }
    while (*type >= 'A' && *type <= 'Z') {
        type++;
    }
    if (*type != ']' || type[1] != ' ' || type[2] == '\0' || type[2] == '\n') {
        return false;
    }
    return true;
}

bool append_entry(
    const char *filepath,
    const char *prefix,
    const char *text
) {
    if (!filepath || !prefix || !text) {
        return false;
    }

    char body[MAX_LINE];
    if (!ledger_sanitize_body(text, body, sizeof(body))) {
        fprintf(stderr, "Error: Empty ledger body after sanitize\n");
        return false;
    }

    FILE *f = fopen(filepath, "a");
    if (!f) {
        fprintf(stderr, "Error: Could not open %s for writing\n", filepath);
        return false;
    }

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", tm);

    if (fprintf(f, "[%s] %s %s\n", ts, prefix, body) < 0) {
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

void play_sound(
    const char *command
) {
    if (command && command[0]) {
        int ret = system(command);
        if (ret != 0) {
            // Sound failed silently (optional: log it)
        }
    }
}

bool ensure_journal(
    char *path_out,
    size_t path_out_sz,
    int *created_out
) {
    if (created_out) {
        *created_out = 0;
    }
    if (!path_out || path_out_sz == 0) {
        return false;
    }
    path_out[0] = '\0';

    if (!ensure_dirs()) {
        return false;
    }

    char *jp = journal_path();
    if (!jp) {
        return false;
    }
    snprintf(path_out, path_out_sz, "%s", jp);

    FILE *f = fopen(path_out, "r");
    if (f) {
        fclose(f);
        return true;
    }

    f = fopen(path_out, "w");
    if (!f) {
        fprintf(stderr, "Error: Could not create journal %s\n", path_out);
        return false;
    }

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char date[64];
    strftime(date, sizeof(date), "%A, %B %d, %Y", tm);
    fprintf(f,
            "# 🌟 Daily Journal — %s\n\n"
            "🙏 Grateful for:\n1. \n2. \n3. \n\n"
            "📚 Learned today:\n\n"
            "🚀 Tomorrow's intention:\n\n"
            "💡 Today's win:\n\n",
            date);
    fclose(f);
    if (created_out) {
        *created_out = 1;
    }
    return true;
}

void open_editor(
    const char *filepath
) {
    if (!filepath) {
        return;
    }

    char *editor = getenv("EDITOR");
    if (!editor || !*editor) {
        editor = "nano";
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, filepath);
    printf("🔧 Opening %s in %s...\n", filepath, editor);
    int ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Warning: Editor exited with code %d\n", ret);
    }
    printf("✅ Saved.\n");
}

// ==================================================================
// Pomodoro session engine (pure state; no sleep/I/O)
// ==================================================================

int pomo_plan_parse(
    const char *spec,
    int *minutes_out,
    int *count_out,
    int max_count
) {
    if (!minutes_out || !count_out || max_count <= 0) {
        return -1;
    }

    if (!spec || !*spec) {
        minutes_out[0] = POMO_DEFAULT_MINUTES;
        *count_out = 1;
        return 0;
    }

    /* Skip leading/trailing whitespace copy into work buffer */
    char buf[MAX_LINE];
    size_t len = strlen(spec);
    if (len >= sizeof(buf)) {
        return -1;
    }
    memcpy(buf, spec, len + 1);

    char *s = trim(buf);
    if (!*s) {
        minutes_out[0] = POMO_DEFAULT_MINUTES;
        *count_out = 1;
        return 0;
    }

    int count = 0;
    char *p = s;
    while (*p) {
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (!*p) {
            break;
        }
        if (!isdigit((unsigned char) *p)) {
            return -1;
        }
        char *end = NULL;
        long v = strtol(p, &end, 10);
        if (end == p || v <= 0 || v > 24 * 60) {
            return -1;
        }
        if (count >= max_count) {
            return -1;
        }
        minutes_out[count++] = (int) v;
        p = end;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == ',') {
            p++;
            /* trailing comma is invalid */
            while (*p == ' ' || *p == '\t') {
                p++;
            }
            if (!*p) {
                return -1;
            }
            continue;
        }
        if (*p != '\0') {
            return -1;
        }
        break;
    }

    if (count == 0) {
        return -1;
    }
    *count_out = count;
    return 0;
}

void pomo_session_init(
    PomoSession *s,
    const int *minutes,
    int count
) {
    if (!s) {
        return;
    }
    memset(s, 0, sizeof(*s));
    if (!minutes || count <= 0) {
        s->segment_minutes[0] = POMO_DEFAULT_MINUTES;
        s->segment_count = 1;
    } else {
        if (count > POMO_MAX_SEGMENTS) {
            count = POMO_MAX_SEGMENTS;
        }
        for (int i = 0; i < count; i++) {
            s->segment_minutes[i] = minutes[i] > 0 ? minutes[i] : POMO_DEFAULT_MINUTES;
        }
        s->segment_count = count;
    }
    s->current_index = 0;
    s->remaining_seconds = s->segment_minutes[0] * 60;
    s->paused = 0;
    s->running = 1;
    s->last_completed_phase = POMO_PHASE_FOCUS;
}

PomoPhase pomo_session_phase(
    const PomoSession *s
) {
    if (!s || s->current_index < 0) {
        return POMO_PHASE_FOCUS;
    }
    /* Even index = focus, odd = break (matches 20,4,20,4) */
    return (s->current_index % 2 == 0) ? POMO_PHASE_FOCUS : POMO_PHASE_BREAK;
}

int pomo_session_segment_seconds(
    const PomoSession *s
) {
    if (!s || s->current_index < 0 || s->current_index >= s->segment_count) {
        return 0;
    }
    return s->segment_minutes[s->current_index] * 60;
}

void pomo_session_apply(
    PomoSession *s,
    PomoControl ctrl
) {
    if (!s || !s->running) {
        return;
    }
    switch (ctrl) {
        case POMO_CTRL_PAUSE_TOGGLE:
            s->paused = !s->paused;
            break;
        case POMO_CTRL_RESTART:
            s->remaining_seconds = pomo_session_segment_seconds(s);
            s->paused = 0;
            break;
        case POMO_CTRL_RESET:
            s->current_index = 0;
            s->remaining_seconds = s->segment_minutes[0] * 60;
            s->paused = 0;
            break;
        case POMO_CTRL_QUIT:
            s->running = 0;
            break;
        case POMO_CTRL_NONE:
        default:
            break;
    }
}

PomoEvent pomo_session_tick(
    PomoSession *s
) {
    if (!s || !s->running || s->paused) {
        return POMO_EVT_NONE;
    }
    if (s->remaining_seconds > 0) {
        s->remaining_seconds--;
    }
    if (s->remaining_seconds > 0) {
        return POMO_EVT_TICK;
    }

    /* Segment finished */
    s->last_completed_phase = pomo_session_phase(s);
    if (s->current_index + 1 >= s->segment_count) {
        s->running = 0;
        return POMO_EVT_PLAN_COMPLETE;
    }
    s->current_index++;
    s->remaining_seconds = s->segment_minutes[s->current_index] * 60;
    return POMO_EVT_SEGMENT_COMPLETE;
}

static void pomo_print_plan_summary(
    const PomoSession *s
) {
    printf("🍅 Plan:");
    for (int i = 0; i < s->segment_count; i++) {
        if (i > 0) {
            printf(" →");
        }
        if (i % 2 == 0) {
            printf(" %dm focus", s->segment_minutes[i]);
        } else {
            printf(" %dm break", s->segment_minutes[i]);
        }
    }
    printf("\n");
}

static void pomo_print_status(
    const PomoSession *s
) {
    int total = pomo_session_segment_seconds(s);
    int remaining = s->remaining_seconds;
    int elapsed = total > 0 ? total - remaining : 0;
    int percent = total > 0 ? (elapsed * 100) / total : 0;
    if (percent > 100) {
        percent = 100;
    }
    int idx = percent / 25;
    if (idx > 4) {
        idx = 4;
    }
    const char *circle[] = {"○", "◔", "◑", "◕", "●"};
    PomoPhase phase = pomo_session_phase(s);
    const char *phase_label = phase == POMO_PHASE_FOCUS ? "FOCUS" : "BREAK";
    const char *icon = phase == POMO_PHASE_FOCUS ? "🍅" : "☕";
    const char *pause_tag = s->paused ? "  ⏸ PAUSED" : "";

    printf("\r\033[K%s %s  %02d:%02d  %3d%%  [%d/%d %s]%s", circle[idx], icon,
           remaining / 60, remaining % 60, percent, s->current_index + 1, s->segment_count,
           phase_label, pause_tag);
    fflush(stdout);
}

static PomoControl pomo_map_key(
    char c
) {
    switch (c) {
        case ' ':
        case 'p':
        case 'P':
            return POMO_CTRL_PAUSE_TOGGLE;
        case 'r':
            return POMO_CTRL_RESTART;
        case 'R':
        case '0':
            return POMO_CTRL_RESET;
        case 'q':
        case 'Q':
        case 3: /* Ctrl-C in raw mode if not signalled */
            return POMO_CTRL_QUIT;
        default:
            return POMO_CTRL_NONE;
    }
}

void pomo_format_log_body(
    const char *context,
    char *out,
    size_t out_sz
) {
    if (!out || out_sz == 0) {
        return;
    }
    if (context && context[0]) {
        snprintf(out, out_sz, "%s", context);
    } else {
        snprintf(out, out_sz, "pomodoro session");
    }
}

void pomo_split_args(
    int argc,
    char **argv,
    char *plan_out,
    size_t plan_sz,
    char *context_out,
    size_t context_sz
) {
    if (plan_out && plan_sz > 0) {
        plan_out[0] = '\0';
    }
    if (context_out && context_sz > 0) {
        context_out[0] = '\0';
    }
    if (argc <= 0 || !argv) {
        return;
    }

    int probe[POMO_MAX_SEGMENTS];
    int nseg = 0;
    int ctx_start = 0;
    if (pomo_plan_parse(argv[0], probe, &nseg, POMO_MAX_SEGMENTS) == 0) {
        if (plan_out && plan_sz > 0) {
            snprintf(plan_out, plan_sz, "%s", argv[0]);
        }
        ctx_start = 1;
    }

    if (!context_out || context_sz == 0) {
        return;
    }
    context_out[0] = '\0';
    for (int i = ctx_start; i < argc; i++) {
        if (!argv[i]) {
            continue;
        }
        if (context_out[0]) {
            strncat(context_out, " ", context_sz - strlen(context_out) - 1);
        }
        strncat(context_out, argv[i], context_sz - strlen(context_out) - 1);
    }
}

static void pomo_on_focus_complete(
    const char *context
) {
    char body[MAX_LINE];
    pomo_format_log_body(context, body, sizeof(body));
    play_sound(sounds.pomo_complete);
    append_entry(data_path(LOG_FILE), "[POMO]", body);
}

void start_pomodoro(
    const char *plan_spec,
    const char *context
) {
    int minutes[POMO_MAX_SEGMENTS];
    int count = 0;
    if (pomo_plan_parse(plan_spec, minutes, &count, POMO_MAX_SEGMENTS) != 0) {
        fprintf(stderr,
                "❌ Invalid pomo plan '%s'. Use minutes (e.g. 25) or a chunk plan "
                "(e.g. 20,4,20,4).\n",
                plan_spec ? plan_spec : "");
        return;
    }

    PomoSession session;
    pomo_session_init(&session, minutes, count);

    if (context && context[0]) {
        printf("🎯 Focus: %s\n", context);
    }
    pomo_print_plan_summary(&session);
    printf("🔥 Let's go!  [space]/p pause  [r] restart segment  [R] reset plan  [q] "
           "quit\n");
    play_sound(sounds.pomo_start);

    int interactive = isatty(STDIN_FILENO);
    struct termios old_tio, new_tio;
    int raw_ok = 0;
    if (interactive) {
        if (tcgetattr(STDIN_FILENO, &old_tio) == 0) {
            new_tio = old_tio;
            new_tio.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
            new_tio.c_cc[VMIN] = 0;
            new_tio.c_cc[VTIME] = 0;
            if (tcsetattr(STDIN_FILENO, TCSANOW, &new_tio) == 0) {
                raw_ok = 1;
            }
        }
    }

    while (session.running) {
        pomo_print_status(&session);

        struct timeval tv;
        if (session.paused) {
            /* Poll often while paused so resume feels snappy */
            tv.tv_sec = 0;
            tv.tv_usec = 200000;
        } else {
            tv.tv_sec = 1;
            tv.tv_usec = 0;
        }

        fd_set rfds;
        FD_ZERO(&rfds);
        int ready = 0;
        if (interactive) {
            FD_SET(STDIN_FILENO, &rfds);
            ready = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
        } else {
            /* Non-TTY: still run the plan (tests / pipes) with wall-clock ticks */
            ready = select(0, NULL, NULL, NULL, &tv);
        }

        if (ready > 0 && interactive && FD_ISSET(STDIN_FILENO, &rfds)) {
            char c = 0;
            if (read(STDIN_FILENO, &c, 1) == 1) {
                PomoControl ctrl = pomo_map_key(c);
                if (ctrl == POMO_CTRL_QUIT) {
                    pomo_session_apply(&session, POMO_CTRL_QUIT);
                    printf("\n⏹ Pomodoro stopped.\n");
                    break;
                }
                if (ctrl != POMO_CTRL_NONE) {
                    pomo_session_apply(&session, ctrl);
                    if (ctrl == POMO_CTRL_PAUSE_TOGGLE) {
                        printf("\n%s\n", session.paused ? "⏸ Paused." : "▶ Resumed.");
                    } else if (ctrl == POMO_CTRL_RESTART) {
                        printf("\n↺ Segment restarted.\n");
                    } else if (ctrl == POMO_CTRL_RESET) {
                        printf("\n⏮ Plan reset to first segment.\n");
                    }
                }
            }
            continue; /* redisplay; don't tick on key-only wake */
        }

        /* Timeout: one second of running time when not paused */
        if (!session.paused) {
            PomoEvent ev = pomo_session_tick(&session);
            if (ev == POMO_EVT_SEGMENT_COMPLETE) {
                if (session.last_completed_phase == POMO_PHASE_FOCUS) {
                    printf("\n✅ Focus segment complete! Logged. 🎉\n");
                    pomo_on_focus_complete(context);
                } else {
                    printf("\n☕ Break over — back to focus when ready.\n");
                }
                if (session.current_index >= 0 &&
                    session.current_index < session.segment_count) {
                    PomoPhase next = pomo_session_phase(&session);
                    printf("→ Next: segment %d/%d · %s (%dm)\n",
                           session.current_index + 1, session.segment_count,
                           next == POMO_PHASE_FOCUS ? "FOCUS" : "BREAK",
                           session.segment_minutes[session.current_index]);
                }
            } else if (ev == POMO_EVT_PLAN_COMPLETE) {
                if (session.last_completed_phase == POMO_PHASE_FOCUS) {
                    printf("\n✅ Focus segment complete! Logged. 🎉\n");
                    pomo_on_focus_complete(context);
                } else {
                    printf("\n☕ Break over.\n");
                }
                if (context && context[0]) {
                    printf("🏁 Full plan complete — %s 🎉\n", context);
                } else {
                    printf("🏁 Full plan complete! Amazing work. 🎉\n");
                }
                break;
            }
        }
    }

    if (raw_ok) {
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    }
}

void list_active_tasks(
    const char *filepath,
    int max_to_show
) {
    if (!filepath) {
        printf("Error: No filepath provided\n");
        return;
    }

    FILE *f = fopen(filepath, "r");
    if (!f) {
        printf("📭 No active tasks yet. Great job staying on top!\n");
        return;
    }

    char line[MAX_LINE];
    int count = 0;
    int shown = 0;
    printf("📋 === Active Tasks ===\n");
    while (fgets(line, sizeof(line), f)) {
        ++count;
        if (max_to_show <= DEFAULT_TASK_LIST_MAX_TO_SHOW || shown < max_to_show) {
            printf("%3d. %s", count, line);
            ++shown;
        }
    }
    fclose(f);

    if (count == 0) {
        printf("No tasks — you're crushing it!\n");
    } else if (max_to_show > 0 && count > max_to_show) {
        printf("    ... and %d more (use 'premflow task list' or 'review --full' to "
               "see all)\n",
               count - max_to_show);
    }
}

bool complete_task(
    const char *filepath,
    int task_num
) {
    if (!filepath || task_num <= 0) {
        return false;
    }

    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: Could not open task file\n");
        return false;
    }

    FILE *tmp = tmpfile();
    if (!tmp) {
        fclose(f);
        fprintf(stderr, "Error: Could not create temporary file\n");
        return false;
    }

    char line[MAX_LINE], task_buf[MAX_LINE] = {0};
    int count = 0, found = 0;

    while (fgets(line, sizeof(line), f)) {
        if (++count == task_num) {
            snprintf(task_buf, sizeof(task_buf), "%s", line);
            found = 1;
            continue;
        }
        fputs(line, tmp);
    }
    fclose(f);

    if (!found) {
        fclose(tmp);
        printf("Task #%d not found.\n", task_num);
        return false;
    }

    f = fopen(filepath, "w");
    if (!f) {
        fclose(tmp);
        fprintf(stderr, "Error: Could not write back to task file\n");
        return false;
    }

    rewind(tmp);
    char c;
    while ((c = fgetc(tmp)) != EOF) {
        fputc(c, f);
    }
    fclose(f);
    fclose(tmp);

    char done_body[MAX_LINE];
    ledger_clean_done_body(task_buf, done_body, sizeof(done_body));
    if (!done_body[0]) {
        snprintf(done_body, sizeof(done_body), "task %d", task_num);
    }

    append_entry(data_path(LOG_FILE), "[DONE]", done_body);
    play_sound(sounds.task_complete);
    printf("✅ Task #%d completed!\n", task_num);
    return true;
}