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

void ensure_dir() {
    char* home = getenv("HOME");
    if (!home) return;
    char dir[256];
    snprintf(dir, sizeof(dir), "%s/%s", home, DATA_DIR);
    char cmd[300];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir);
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
        printf("No tasks yet. Add some with 'task add <text>'\n");
        return;
    }
    char line[MAX_LINE];
    printf("=== Your Tasks ===\n");
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        printf("%3d. %s", ++count, line);
    }
    fclose(f);
    if (count == 0) printf("No tasks yet.\n");
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
            continue;  // remove this task
        }
        fputs(line, temp);
    }
    fclose(f);

    if (!found) {
        fclose(temp);
        printf("Task #%d not found.\n", task_num);
        return;
    }

    // Rewrite todo file without the completed task
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
    if (!editor || strlen(editor) == 0) {
        editor = "nano";  // fallback (change to "vim" if you prefer)
    }
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, filepath);
    printf("🔧 Opening %s in your %s editor (full TUI editing)...\n", filepath, editor);
    system(cmd);
    printf("✅ File saved. premflow is ready for your next command!\n");
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
        printf("=== premflow - Your Daily Time-Saving Companion ===\n\n");
        printf("Usage:\n");
        printf("  premflow note \"your idea or thought\"\n");
        printf("  premflow task add \"new task here\"\n");
        printf("  premflow task list\n");
        printf("  premflow task done <number>\n");
        printf("  premflow pomo <minutes>     (default 25)\n");
        printf("  premflow edit               ← Full TUI editor on your notes\n");
        printf("  premflow edit todo          ← Full TUI editor on your tasks\n");
        printf("  premflow review\n");
        printf("\nBuilt for Peramanathan Sathyamoorthy's next extraordinary chapter 🚀\n");
        printf("One command = capture + focus + complete + **edit in real TUI** + review.\n");
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
            if (num > 0) {
                complete_task(get_data_path(TODO_FILE), num);
            } else {
                printf("Usage: premflow task done <number>\n");
            }
        }
        else {
            printf("Unknown task command. Use: add / list / done\n");
        }
    }
    else if (strcmp(cmd, "pomo") == 0) {
        int mins = (argc > 2) ? atoi(argv[2]) : 25;
        pomodoro(mins);
    }
    else if (strcmp(cmd, "edit") == 0) {
        const char* target_file = (argc > 2 && strcmp(argv[2], "todo") == 0) ? TODO_FILE : LOG_FILE;
        launch_editor(get_data_path(target_file));
    }
    else if (strcmp(cmd, "review") == 0) {
        printf("=== Daily Review - Let's see your wins, Prem! ===\n");
        char cmd_str[512];
        snprintf(cmd_str, sizeof(cmd_str), "tail -n 30 %s 2>/dev/null || echo 'No entries yet. Start capturing!'", get_data_path(LOG_FILE));
        system(cmd_str);
        printf("\nActive Tasks:\n");
        list_todos(get_data_path(TODO_FILE));
    }
    else {
        printf("Unknown command. Run 'premflow' for help.\n");
    }

    return 0;
}