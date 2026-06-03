#include "premflow.h"
#include <stdlib.h>
#include <string.h>

#define C_RESET "\033[0m"
#define C_BOLD "\033[1m"
#define C_DIM "\033[2m"
#define C_GREEN "\033[32m"
#define C_YELLOW "\033[33m"
#define C_BLUE "\033[34m"
#define C_CYAN "\033[36m"
#define C_MAG "\033[35m"

void show_help(
    void
) {
    puts("🌟 === premflow — Tiny. Clean. Powerful. === 🌟");
    puts("\n📋 Commands:");
    puts("  📝 note \"idea\"");
    puts("  ✅ task add \"buy milk\"");
    puts("  📋 task list");
    puts("  ✔️  task done <n>");
    puts("  🏆 win \"great work\"");
    puts("  📖 journal");
    puts("  🍅 pomo [minutes]");
    puts("  ✏️  edit [todo]");
    puts("  🔍 search \"keyword\"");
    puts("  📊 stats");
    puts("  📅 review          (smart: highlights + pending todos first)");
    puts("  📅 review --full   (raw dump of everything)");
    puts("  ⚙️  config sound");
}

void show_stats(
    void
) {
    char *log = data_path(LOG_FILE);
    if (!log) {
        return;
    }

    printf("🏆 premflow Stats Dashboard 🏆\n");
    printf("══════════════════════════════════\n");
    char buf[512];
    snprintf(buf, sizeof(buf),
             "echo '   🍅 Pomos completed : '; grep -c '\\[POMO\\]' \"%s\" || echo 0",
             log);
    system(buf);
    snprintf(buf, sizeof(buf),
             "echo '   ✅ Tasks completed : '; grep -c '\\[DONE\\]' \"%s\" || echo 0",
             log);
    system(buf);
    snprintf(buf, sizeof(buf),
             "echo '   📝 Notes logged    : '; grep -c '\\[NOTE\\]' \"%s\" || echo 0",
             log);
    system(buf);
    snprintf(buf, sizeof(buf),
             "echo '   🏆 Wins & wins     : '; grep -c '\\[WIN\\]'  \"%s\" || echo 0",
             log);
    system(buf);
    printf("══════════════════════════════════\n");
}

void show_review(
    int full
) {
    printf("📅 === Daily Review — Let's celebrate your wins! === 📅\n");
    char *log_tmp = data_path(LOG_FILE);
    char log_path[512] = {0};
    if (log_tmp) {
        snprintf(log_path, sizeof(log_path), "%s", log_tmp);
    }
    char *todo_tmp = data_path(TODO_FILE);
    char todo_path[512] = {0};
    if (todo_tmp) {
        snprintf(todo_path, sizeof(todo_path), "%s", todo_tmp);
    }
    if (!log_path[0] || !todo_path[0]) {
        return;
    }

    if (full) {
        printf(C_DIM " (full mode — explicit raw log + all todos)\n" C_RESET);
        fflush(stdout);
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "tail -n 60 %s 2>/dev/null || echo 'No entries yet'",
                 log_path);
        system(cmd);
        puts("\n📋 Active Tasks (all):");
        {
            const int max_to_show = DEFAULT_TASK_LIST_MAX_TO_SHOW;
            list_active_tasks(todo_path, max_to_show);
        }
        return;
    }

    /* === SMART DEFAULT: curated, highlighted, high-priority first === */
    printf(
        C_DIM
        " (smart view — pomos grouped, focus on signal. Add --full for raw.)\n" C_RESET
    );

    /* High priority: pending todos always front-and-center */
    printf("\n" C_BOLD C_YELLOW "📋 Pending Priorities" C_RESET
           " — tackle these first\n");
    {
        const int max_to_show = REVIEW_TASK_LIST_MAX_TO_SHOW;
        list_active_tasks(todo_path, max_to_show);
    }

    /* Read log, categorize, suppress low-value POMO spam by default */
    FILE *f = fopen(log_path, "r");
    if (!f) {
        puts("   (no log entries yet)");
        return;
    }

#define MAX_HL 12
    char hl_wins[MAX_HL][MAX_LINE];
    char hl_notes[MAX_HL][MAX_LINE];
    char hl_dones[MAX_HL][MAX_LINE];
    int nwin = 0, nnote = 0, ndone = 0, npomo = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "[POMO]")) {
            npomo++;
            continue;
        }
        if (strstr(line, "[WIN]") && nwin < MAX_HL) {
            snprintf(hl_wins[nwin++], MAX_LINE, "%s", line);
        } else if (strstr(line, "[NOTE]") && nnote < MAX_HL) {
            snprintf(hl_notes[nnote++], MAX_LINE, "%s", line);
        } else if (strstr(line, "[DONE]") && ndone < MAX_HL) {
            snprintf(hl_dones[ndone++], MAX_LINE, "%s", line);
        }
    }
    fclose(f);

    /* Visual sections with highlights */
    puts("");
    if (nwin > 0) {
        printf(C_GREEN C_BOLD "🏆 Wins" C_RESET "\n");
        int start = (nwin > 5 ? nwin - 5 : 0);
        for (int i = start; i < nwin; i++) {
            char *content = strstr(hl_wins[i], "[WIN]");
            if (content) {
                content += 5;
            } else {
                content = hl_wins[i];
            }
            /* strip leading ts if present */
            char *tsend = strstr(content, "] ");
            if (tsend) {
                content = tsend + 2;
            }
            printf("   ✨ %s", content);
        }
    }

    if (nnote > 0) {
        printf(C_CYAN C_BOLD "💡 Notes & Ideas" C_RESET "\n");
        int start = (nnote > 5 ? nnote - 5 : 0);
        for (int i = start; i < nnote; i++) {
            char *content = strstr(hl_notes[i], "[NOTE]");
            if (content) {
                content += 6;
            } else {
                content = hl_notes[i];
            }
            char *tsend = strstr(content, "] ");
            if (tsend) {
                content = tsend + 2;
            }
            printf("   📝 %s", content);
        }
    }

    if (ndone > 0) {
        printf(C_YELLOW C_BOLD "✅ Recent Completions" C_RESET "\n");
        int start = (ndone > 5 ? ndone - 5 : 0);
        for (int i = start; i < ndone; i++) {
            char *content = strstr(hl_dones[i], "[DONE]");
            if (content) {
                content += 6;
            } else {
                content = hl_dones[i];
            }
            /* clean old embedded prefixes from task completion logs */
            char *emb = strstr(content, "[TODO]");
            if (emb) {
                content = emb + 6;
            }
            char *tsend = strstr(content, "] ");
            if (tsend && tsend < content + 30) {
                content = tsend + 2; /* avoid eating real text */
            }
            content = ltrim(content);
            printf("   ✔️  %s", content);
        }
    }

    /* Always surface the volume of focus work, but de-emphasized */
    if (npomo > 0) {
        printf(
            "\n" C_DIM
            "🍅 %d pomodoro sessions (focus work logged — hidden by default)\n" C_RESET,
            npomo
        );
    }

    puts("\n" C_DIM
         "Use: premflow review --full   for complete history + every entry" C_RESET);
    puts(C_DIM "     premflow task list          for full pending list" C_RESET);
}

void show_search(
    const char *term
) {
    if (!term) {
        return;
    }

    char *log_tmp = data_path(LOG_FILE);
    char log_path[512] = {0};
    if (log_tmp) {
        snprintf(log_path, sizeof(log_path), "%s", log_tmp);
    }
    char *todo_tmp = data_path(TODO_FILE);
    char todo_path[512] = {0};
    if (todo_tmp) {
        snprintf(todo_path, sizeof(todo_path), "%s", todo_tmp);
    }
    if (!log_path[0] || !todo_path[0]) {
        return;
    }

    printf("🔎 Search Results for \"%s\":\n\n", term);

    char cmd[1024];
    // Search both files, show filename, and deduplicate results
    snprintf(cmd, sizeof(cmd),
             "{ grep -i -H --color=always \"%s\" \"%s\" \"%s\" 2>/dev/null || true; } "
             "| sort -u || echo '   No matches found'",
             term, log_path, todo_path);
    system(cmd);
}