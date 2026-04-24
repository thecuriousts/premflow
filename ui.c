#include "premflow.h"
#include <stdlib.h>
#include <string.h>

void show_help(void) {
    puts("=== premflow — Tiny. Clean. Powerful. ===");
    puts("\nCommands:");
    puts("  note \"idea\"");
    puts("  task add \"buy milk\"");
    puts("  task list");
    puts("  task done <n>");
    puts("  win \"great work\"");
    puts("  journal");
    puts("  pomo [minutes]");
    puts("  edit [todo]");
    puts("  search \"keyword\"");
    puts("  stats");
    puts("  review");
    puts("  config sound");
}

void show_stats(void) {
    char* log = data_path(LOG_FILE);
    if (!log) return;

    printf("🏆 premflow Stats\n================\n");
    char buf[512];
    snprintf(buf, sizeof(buf), "echo '   Pomos  : '; grep -c '\\[POMO\\]' \"%s\" || echo 0", log); system(buf);
    snprintf(buf, sizeof(buf), "echo '   Tasks  : '; grep -c '\\[DONE\\]' \"%s\" || echo 0", log); system(buf);
    snprintf(buf, sizeof(buf), "echo '   Notes  : '; grep -c '\\[NOTE\\]' \"%s\" || echo 0", log); system(buf);
    snprintf(buf, sizeof(buf), "echo '   Wins   : '; grep -c '\\[WIN\\]'  \"%s\" || echo 0", log); system(buf);
}

void show_review(void) {
    printf("=== Daily Review ===\n");
    char* log = data_path(LOG_FILE);
    if (!log) return;

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "tail -n 30 %s 2>/dev/null || echo 'No entries yet'", log);
    system(cmd);
    puts("\nActive Tasks:");
    list_active_tasks(data_path(TODO_FILE));
}

void show_search(const char* term) {
    if (!term) return;

    char* log = data_path(LOG_FILE);
    char* todo = data_path(TODO_FILE);
    if (!log || !todo) return;

    printf("🔎 Results for \"%s\":\n\n", term);

    char cmd[1024];
    // Search both files at once and show filename
    snprintf(cmd, sizeof(cmd),
        "grep -i -H --color=always \"%s\" \"%s\" \"%s\" 2>/dev/null || echo '   No matches found'",
        term, log, todo);
    system(cmd);
}
