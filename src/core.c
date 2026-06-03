#include "premflow.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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

bool append_entry(
    const char *filepath,
    const char *prefix,
    const char *text
) {
    if (!filepath || !prefix || !text) {
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

    if (fprintf(f, "[%s] %s %s\n", ts, prefix, text) < 0) {
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

void start_pomodoro(
    int minutes
) {
    if (minutes <= 0) {
        minutes = 25;
    }
    int total_seconds = minutes * 60;

    printf("🍅 Pomodoro started — %d min deep focus! Let's go! 🔥\n", minutes);
    play_sound(sounds.pomo_start);

    // Beautiful Unicode circle progress (quarter steps)
    const char *circle[] = {
        "○", "◔", "◑", "◕", "●" // 0%, 25%, 50%, 75%, 100%
    };

    for (int remaining = total_seconds; remaining > 0; remaining--) {
        int elapsed = total_seconds - remaining;
        int percent = (elapsed * 100) / total_seconds;

        // Choose circle based on percentage
        int idx = percent / 25;
        if (idx > 4) {
            idx = 4;
        }

        printf("\r%s 🍅  %02d:%02d  %3d%%", circle[idx], remaining / 60, remaining % 60,
               percent);
        fflush(stdout);
        sleep(1);
    }

    printf("\n✅ Pomodoro complete! Amazing focus, Prem! 🎉\n");
    play_sound(sounds.pomo_complete);
    append_entry(data_path(LOG_FILE), "[POMO]", "pomodoro session");
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

    char *clean = trim(task_buf);
    /* robust strip of leading timestamp + [TODO] prefix from stored todo line */
    char *todo_tag = strstr(clean, "[TODO]");
    if (todo_tag) {
        clean = todo_tag + 6;
    } else {
        /* skip [ts] [TYPE] prefix: locate second ']' */
        char *p = strchr(clean, ']');
        if (p) {
            p = strchr(p + 1, ']');
        }
        if (p) {
            clean = p + 1;
        }
    }
    clean = trim(clean);

    append_entry(data_path(LOG_FILE), "[DONE]", clean);
    play_sound(sounds.task_complete);
    printf("✅ Task #%d completed!\n", task_num);
    return true;
}