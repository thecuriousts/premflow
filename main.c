#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>  // for sleep()

#define MAX_LINE 1024
#define DATA_DIR ".premflow"
#define LOG_FILE "log.txt"
#define TODO_FILE "todo.txt"

char* get_data_path(const char* filename) {
    char* home = getenv("HOME");
    if (!home) return NULL;
    static char path[512];
    snprintf(path, sizeof(path), "%s/%s/%s", home, DATA_DIR, filename);
    return path;
}

char* get_journal_path() {
    char* home = getenv("HOME");
    if (!home) return NULL;
    static char path[512];
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char date[32];
    strftime(date, sizeof(date), "%Y-%m-%d", tm);
    snprintf(path, sizeof(path), "%s/%s/journal/journal-%s.txt", home, DATA_DIR, date);
    return path;
}

void ensure_dir() {
    char* home = getenv("HOME");
    if (!home) return;
    char dir[256];
    snprintf(dir, sizeof(dir), "%s/%s", home, DATA_DIR);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/journal", dir);
    system(cmd);
}

void append_to_file(const char* filepath, const char* prefix, const char* content) {
    FILE* f = fopen(filepath, "a");
    if (!f) {
        printf("Error opening %s\n", filepath);
        return;
    }
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", tm);
    fprintf(f, "[%s] %s %s\n", ts, prefix, content);
    fclose(f);
    printf("✓ Saved: %s\n", content);
}

void list_todos(const char* filepath) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        printf("No active tasks. Add some with 'task add <text>'\n");
        return;
    }
    char line[MAX_LINE];
    printf("=== Your Active Tasks ===\n");
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        printf("%3d. %s", ++count, line);
    }
    fclose(f);
    if (count == 0) printf("No active tasks yet. Great job!\n");
}

void complete_task(const char* filepath, int task_num) {
    FILE* f = fopen(filepath, "r");
    if (!f) {
        printf("No tasks found.\n");
        return;
    }
    FILE* temp = tmpfile();
    if (!temp) {
        fclose(f);
        printf("Error creating temporary file.\n");
        return;
    }
    char line[MAX_LINE];
    char completed_task[MAX_LINE] = {0};
    int count = 0;
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        count++;
        if (count == task_num) {
            strncpy(completed_task, line, sizeof(completed_task)-1);
            found = 1;
            continue;
        }
        fputs(line, temp);
    }
    fclose(f);
    if (!found) {
        fclose(temp);
        printf("Task #%d not found.\n", task_num);
        return;
    }
    f = fopen(filepath, "w");
    if (f) {
        rewind(temp);
        char c;
        while ((c = fgetc(temp)) != EOF) fputc(c, f);
        fclose(f);
    }
    fclose(temp);
    if (strlen(completed_task) > 0) {
        char* clean = completed_task;
        while (*clean && (*clean == ' ' || *clean == '\t' || *clean == '\n')) clean++;
        if (strncmp(clean, "[TODO]", 6) == 0) clean += 6;
        while (*clean && (*clean == ' ' || *clean == '\t')) clean++;
        append_to_file(get_data_path(LOG_FILE), "[DONE]", clean);
        printf("✅ Task #%d marked COMPLETE & moved to your wins log!\n", task_num);
    }
}

void launch_editor(const char* filepath) {
    char* editor = getenv("EDITOR");
    if (!editor || strlen(editor) == 0) editor = "nano";
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, filepath);
    printf("🔧 Opening %s in %s (full TUI editing)...\n", filepath, editor);
    system(cmd);
    printf("✅ File saved. premflow is ready!\n");
}

void launch_journal() {
    char* filepath = get_journal_path();
    FILE* f = fopen(filepath, "r");
    if (!f) {
        f = fopen(filepath, "w");
        if (f) {
            time_t now = time(NULL);
            struct tm *tm = localtime(&now);
            char date_str[64];
            strftime(date_str, sizeof(date_str), "%A, %B %d, %Y", tm);
            fprintf(f, "# 🌟 Daily Journal — %s\n\n", date_str);
            fprintf(f, "🙏 Grateful for (3 things):\n1. \n2. \n3. \n\n");
            fprintf(f, "📚 What I learned today:\n\n");
            fprintf(f, "🚀 Top intention for tomorrow:\n\n");
            fprintf(f, "💡 One win from today:\n\n");
            fclose(f);
            printf("📖 New daily journal created with template!\n");
        }
    } else {
        fclose(f);
    }
    launch_editor(filepath);
}

void search_logs(const char* term) {
    char* logpath = get_data_path(LOG_FILE);
    char* todopath = get_data_path(TODO_FILE);
    char cmd[1024];
    printf("🔎 Search results for \"%s\":\n\n", term);
    snprintf(cmd, sizeof(cmd), "grep -i --color=always \"%s\" \"%s\" 2>/dev/null || echo '   No matches in notes'", term, logpath);
    system(cmd);
    printf("\n");
    snprintf(cmd, sizeof(cmd), "grep -i --color=always \"%s\" \"%s\" 2>/dev/null || echo '   No matches in tasks'", term, todopath);
    system(cmd);
}

void show_stats() {
    char* logpath = get_data_path(LOG_FILE);
    printf("🏆 premflow Stats Dashboard — Peramanathan Sathyamoorthy\n");
    printf("==================================================\n");
    char cmd[1024];
    printf("\n📊 Lifetime Highlights:\n");
    snprintf(cmd, sizeof(cmd), "echo '   Pomos completed     : '; grep -c '\\[POMO\\]' \"%s\" 2>/dev/null || echo '0'", logpath);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "echo '   Tasks completed     : '; grep -c '\\[DONE\\]' \"%s\" 2>/dev/null || echo '0'", logpath);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "echo '   Notes & ideas logged: '; grep -c '\\[NOTE\\]' \"%s\" 2>/dev/null || echo '0'", logpath);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "echo '   Wins & gratitudes   : '; grep -c '\\[WIN\\]' \"%s\" 2>/dev/null || echo '0'", logpath);
    system(cmd);
    printf("\n📅 Today's journal: ");
    snprintf(cmd, sizeof(cmd), "ls \"%s/journal/\" 2>/dev/null | grep -c journal-$(date +%%Y-%%m-%%d) || echo 'not started yet'", get_data_path(""));
    system(cmd);
    printf("\n🔥 Keep the streak alive, Prem! 🚀\n");
}

void pomodoro(int minutes) {
    if (minutes <= 0) minutes = 25;
    int seconds = minutes * 60;
    printf("🍅 Pomodoro started for %d minutes. Focus time, Prem!\n", minutes);
    for (int i = seconds; i > 0; i--) {
        printf("\rTime left: %02d:%02d ", i/60, i%60);
        fflush(stdout);
        sleep(1);
    }
    printf("\n✅ Pomodoro complete! Great work, Prem. Session logged.\n");
    append_to_file(get_data_path(LOG_FILE), "[POMO]", "Completed focused work session");
}

int main(int argc, char* argv[]) {
    ensure_dir();

    if (argc < 2) {
        printf("=== premflow — Your Daily Productivity Powerhouse ===\n\n");
        printf("Usage:\n");
        printf("  premflow note \"idea\"\n");
        printf("  premflow task add \"new task\"\n");
        printf("  premflow task list\n");
        printf("  premflow task done <number>\n");
        printf("  premflow win \"what went great today\"\n");
        printf("  premflow journal                  ← Full TUI daily journal + template\n");
        printf("  premflow pomo <minutes>\n");
        printf("  premflow edit [todo]              ← TUI editor for notes or tasks\n");
        printf("  premflow search \"keyword\"\n");
        printf("  premflow stats                    ← Personal productivity dashboard\n");
        printf("  premflow review\n");
        printf("\nBuilt exclusively for Simple and Power Efficient User 🚀\n");
        printf("One tiny C binary. Zero bloat. Maximum daily time saved.\n");
        return 0;
    }

    char* cmd = argv[1];

    if (strcmp(cmd, "note") == 0 && argc > 2) {
        char note[MAX_LINE] = {0};
        for (int i = 2; i < argc; i++) {
            strcat(note, argv[i]);
            if (i < argc - 1) strcat(note, " ");
        }
        append_to_file(get_data_path(LOG_FILE), "[NOTE]", note);
    }
    else if (strcmp(cmd, "task") == 0 && argc > 2) {
        if (strcmp(argv[2], "add") == 0 && argc > 3) {
            char task[MAX_LINE] = {0};
            for (int i = 3; i < argc; i++) {
                strcat(task, argv[i]);
                if (i < argc - 1) strcat(task, " ");
            }
            append_to_file(get_data_path(TODO_FILE), "[TODO]", task);
        }
        else if (strcmp(argv[2], "list") == 0) {
            list_todos(get_data_path(TODO_FILE));
        }
        else if (strcmp(argv[2], "done") == 0 && argc > 3) {
            int num = atoi(argv[3]);
            if (num > 0) complete_task(get_data_path(TODO_FILE), num);
            else printf("Usage: premflow task done <number>\n");
        }
        else {
            printf("Unknown task command. Use: add / list / done\n");
        }
    }
    else if (strcmp(cmd, "win") == 0 && argc > 2) {
        char win[MAX_LINE] = {0};
        for (int i = 2; i < argc; i++) {
            strcat(win, argv[i]);
            if (i < argc - 1) strcat(win, " ");
        }
        append_to_file(get_data_path(LOG_FILE), "[WIN]", win);
    }
    else if (strcmp(cmd, "journal") == 0) {
        launch_journal();
    }
    else if (strcmp(cmd, "pomo") == 0) {
        int mins = (argc > 2) ? atoi(argv[2]) : 25;
        pomodoro(mins);
    }
    else if (strcmp(cmd, "edit") == 0) {
        const char* target = (argc > 2 && strcmp(argv[2], "todo") == 0) ? TODO_FILE : LOG_FILE;
        launch_editor(get_data_path(target));
    }
    else if (strcmp(cmd, "search") == 0 && argc > 2) {
        search_logs(argv[2]);
    }
    else if (strcmp(cmd, "stats") == 0) {
        show_stats();
    }
    else if (strcmp(cmd, "review") == 0) {
        printf("=== Daily Review — Let's celebrate your wins, Prem! ===\n");
        char cmd_str[512];
        snprintf(cmd_str, sizeof(cmd_str), "tail -n 30 %s 2>/dev/null || echo 'No entries yet.'", get_data_path(LOG_FILE));
        system(cmd_str);
        printf("\nActive Tasks:\n");
        list_todos(get_data_path(TODO_FILE));
    }
    else {
        printf("Unknown command. Run 'premflow' for full help.\n");
    }

    return 0;
}