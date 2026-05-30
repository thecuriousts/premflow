#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void pf_handle_cmd(Cmd cmd, Msg *result_msg) {
    (void)result_msg;
    if (!cmd) return;

    CmdData *cd = (CmdData *)cmd;
    if (cd->type != CMD_CUSTOM || !cd->data) return;

    EffectPayload *p = (EffectPayload *)cd->data;
    PremflowRuntime *rt = premflow_runtime_get();

    switch (p->kind) {
    case EFFECT_APPEND_NOTE: {
        char *path = data_path(LOG_FILE);
        if (!path || !append_entry(path, "[NOTE]", p->text)) {
            rt->exit_code = 1;
            strncpy(rt->error, "Failed to save note", sizeof(rt->error) - 1);
        }
        break;
    }
    case EFFECT_APPEND_TODO: {
        char *path = data_path(TODO_FILE);
        if (!path || !append_entry(path, "[TODO]", p->text)) {
            rt->exit_code = 1;
            strncpy(rt->error, "Failed to add task", sizeof(rt->error) - 1);
        }
        break;
    }
    case EFFECT_APPEND_WIN: {
        char *path = data_path(LOG_FILE);
        if (!path || !append_entry(path, "[WIN]", p->text)) {
            rt->exit_code = 1;
            strncpy(rt->error, "Failed to log win", sizeof(rt->error) - 1);
        }
        break;
    }
    case EFFECT_TASK_DONE: {
        char *path = data_path(TODO_FILE);
        if (!path || !complete_task(path, p->task_num)) {
            rt->exit_code = 1;
        }
        break;
    }
    case EFFECT_JOURNAL: {
        char *path = journal_path();
        if (!path) {
            rt->exit_code = 1;
            break;
        }
        FILE *f = fopen(path, "r");
        if (!f) {
            f = fopen(path, "w");
            if (f) {
                time_t now = time(NULL);
                struct tm *tm = localtime(&now);
                char date[64];
                strftime(date, sizeof(date), "%A, %B %d, %Y", tm);
                fprintf(f, "# 🌟 Daily Journal — %s\n\n"
                           "🙏 Grateful for:\n1. \n2. \n3. \n\n"
                           "📚 Learned today:\n\n"
                           "🚀 Tomorrow's intention:\n\n"
                           "💡 Today's win:\n\n",
                        date);
                fclose(f);
                printf("📖 New journal created!\n");
            }
        } else {
            fclose(f);
        }
        open_editor(path);
        break;
    }
    case EFFECT_POMO:
        start_pomodoro(p->pomo_minutes);
        break;
    case EFFECT_EDIT_LOG: {
        char *path = data_path(LOG_FILE);
        if (!path) {
            rt->exit_code = 1;
        } else {
            open_editor(path);
        }
        break;
    }
    case EFFECT_EDIT_TODO: {
        char *path = data_path(TODO_FILE);
        if (!path) {
            rt->exit_code = 1;
        } else {
            open_editor(path);
        }
        break;
    }
    case EFFECT_CONFIG_SOUND: {
        char *path = data_path(CONFIG_FILE);
        if (!path) {
            rt->exit_code = 1;
            break;
        }
        if (access(path, F_OK) != 0) {
            create_config_template(path);
        }
        open_editor(path);
        break;
    }
    default:
        break;
    }
}
