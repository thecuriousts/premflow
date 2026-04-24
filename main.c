#include "premflow.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (!ensure_dirs()) {
        fprintf(stderr, "Failed to initialize directories. Exiting.\n");
        return 1;
    }
    read_config();

    if (argc < 2) {
        show_help();
        return 0;
    }

    char* cmd = argv[1];
    int exit_code = 0;

    if (strcmp(cmd, "note") == 0 && argc > 2) {
        char buf[MAX_LINE] = {0};
        for (int i = 2; i < argc; i++) {
            if (i > 2) strcat(buf, " ");
            strcat(buf, argv[i]);
        }
        if (!append_entry(data_path(LOG_FILE), "[NOTE]", buf)) {
            exit_code = 1;
        } else {
            printf("✓ Note saved\n");
        }
    }
    else if (strcmp(cmd, "task") == 0 && argc > 2) {
        if (strcmp(argv[2], "add") == 0 && argc > 3) {
            char buf[MAX_LINE] = {0};
            for (int i = 3; i < argc; i++) {
                if (i > 3) strcat(buf, " ");
                strcat(buf, argv[i]);
            }
            if (!append_entry(data_path(TODO_FILE), "[TODO]", buf)) {
                exit_code = 1;
            } else {
                printf("✓ Task added\n");
            }
        }
        else if (strcmp(argv[2], "list") == 0) {
            list_active_tasks(data_path(TODO_FILE));
        }
        else if (strcmp(argv[2], "done") == 0 && argc > 3) {
            if (!complete_task(data_path(TODO_FILE), atoi(argv[3]))) {
                exit_code = 1;
            }
        }
    }
    else if (strcmp(cmd, "win") == 0 && argc > 2) {
        char buf[MAX_LINE] = {0};
        for (int i = 2; i < argc; i++) {
            if (i > 2) strcat(buf, " ");
            strcat(buf, argv[i]);
        }
        if (!append_entry(data_path(LOG_FILE), "[WIN]", buf)) {
            exit_code = 1;
        } else {
            printf("✓ Win logged\n");
        }
    }
    else if (strcmp(cmd, "journal") == 0) {
        char* path = journal_path();
        if (!path) {
            exit_code = 1;
        } else {
            FILE* f = fopen(path, "r");
            if (!f) {
                f = fopen(path, "w");
                if (f) {
                    time_t now = time(NULL);
                    struct tm* tm = localtime(&now);
                    char date[64];
                    strftime(date, sizeof(date), "%A, %B %d, %Y", tm);
                    fprintf(f, "# 🌟 Daily Journal — %s\n\n"
                               "🙏 Grateful for:\n1. \n2. \n3. \n\n"
                               "📚 Learned today:\n\n"
                               "🚀 Tomorrow's intention:\n\n"
                               "💡 Today's win:\n\n", date);
                    fclose(f);
                    printf("📖 New journal created!\n");
                }
            } else {
                fclose(f);
            }
            open_editor(path);
        }
    }
    else if (strcmp(cmd, "pomo") == 0) {
        start_pomodoro(argc > 2 ? atoi(argv[2]) : 25);
    }
    else if (strcmp(cmd, "edit") == 0) {
        const char* target = (argc > 2 && strcmp(argv[2], "todo") == 0) ? TODO_FILE : LOG_FILE;
        char* path = data_path(target);
        if (path) open_editor(path);
        else exit_code = 1;
    }
    else if (strcmp(cmd, "search") == 0 && argc > 2) {
        show_search(argv[2]);
    }
    else if (strcmp(cmd, "stats") == 0) {
        show_stats();
    }
    else if (strcmp(cmd, "review") == 0) {
        show_review();
    }
    else if (strcmp(cmd, "config") == 0 && argc > 2 && strcmp(argv[2], "sound") == 0) {
        char* path = data_path(CONFIG_FILE);
        if (!path) {
            exit_code = 1;
        } else {
            if (access(path, F_OK) != 0)
                create_config_template(path);
            open_editor(path);
        }
    }
    else {
        printf("Unknown command. Run 'premflow' for help.\n");
        exit_code = 1;
    }

    return exit_code;
}
