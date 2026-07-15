#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PremflowRuntime g_runtime;

void premflow_runtime_init(
    PremflowRuntime *rt
) {
    if (!rt) {
        return;
    }
    rt->exit_code = 0;
    rt->error[0] = '\0';
    g_runtime = *rt;
}

PremflowRuntime *premflow_runtime_get(
    void
) {
    return &g_runtime;
}

static PremflowModel *model_new(
    int exit_code,
    DisplayKind display,
    const char *feedback
) {
    PremflowModel *m = calloc(1, sizeof(PremflowModel));
    if (!m) {
        return NULL;
    }

    m->exit_code = exit_code;
    m->display = display;
    if (feedback) {
        strncpy(m->feedback, feedback, sizeof(m->feedback) - 1);
    }
    return m;
}

static PremflowModel *model_with_search(
    const char *term
) {
    PremflowModel *m = model_new(0, DISPLAY_SEARCH, NULL);
    if (m && term) {
        snprintf(m->search_term, sizeof(m->search_term), "%s", term);
    }
    return m;
}

static void fail_runtime(
    const char *msg
) {
    g_runtime.exit_code = 1;
    if (msg) {
        strncpy(g_runtime.error, msg, sizeof(g_runtime.error) - 1);
    }
}

static void emit_effect(
    Cmd *cmds_out,
    size_t *n,
    EffectKind kind,
    const char *text,
    int task_num,
    int pomo_minutes,
    const char *pomo_plan
) {
    EffectPayload *p = malloc(sizeof(EffectPayload));
    if (!p) {
        fail_runtime("Out of memory");
        return;
    }

    memset(p, 0, sizeof(*p));
    p->kind = kind;
    p->task_num = task_num;
    p->pomo_minutes = pomo_minutes;
    if (text) {
        strncpy(p->text, text, sizeof(p->text) - 1);
    }
    if (pomo_plan) {
        strncpy(p->pomo_plan, pomo_plan, sizeof(p->pomo_plan) - 1);
    }
    cmds_out[*n] = elomaxz_make_cmd(CMD_CUSTOM, p, sizeof(EffectPayload));
    if (cmds_out[*n]) {
        (*n)++;
    } else {
        free(p);
        fail_runtime("Out of memory");
    }
}

Model pf_init(
    void
) {
    return (Model) model_new(0, DISPLAY_NONE, NULL);
}

Model pf_update(
    Model current,
    Msg msg,
    Cmd *cmds_out,
    size_t *num_cmds_out
) {
    (void) current;
    *num_cmds_out = 0;
    g_runtime.exit_code = 0;
    g_runtime.error[0] = '\0';

    PremflowMsg *m = (PremflowMsg *) msg;
    if (!m) {
        return (Model) model_new(1, DISPLAY_NONE, NULL);
    }

    switch (m->type) {
        case PF_MSG_HELP:
            return (Model) model_new(0, DISPLAY_HELP, NULL);

        case PF_MSG_UNKNOWN:
            fail_runtime(NULL);
            return (Model) model_new(1, DISPLAY_NONE,
                                     "Unknown command. Run 'premflow' for help.");

        case PF_MSG_NOTE:
            if (!m->text[0]) {
                fail_runtime("Note text required");
                return (Model) model_new(1, DISPLAY_NONE, NULL);
            }

            emit_effect(cmds_out, num_cmds_out, EFFECT_APPEND_NOTE, m->text, 0, 0, NULL);
            return (Model) model_new(0, DISPLAY_NONE, "✓ Note saved");

        case PF_MSG_TASK_ADD:
            if (!m->text[0]) {
                fail_runtime("Task text required");
                return (Model) model_new(1, DISPLAY_NONE, NULL);
            }

            emit_effect(cmds_out, num_cmds_out, EFFECT_APPEND_TODO, m->text, 0, 0, NULL);
            return (Model) model_new(0, DISPLAY_NONE, "✓ Task added");

        case PF_MSG_TASK_LIST:
            return (Model) model_new(0, DISPLAY_TASK_LIST, NULL);

        case PF_MSG_TASK_DONE:
            if (m->task_num <= 0) {
                fail_runtime("Invalid task number");
                return (Model) model_new(1, DISPLAY_NONE, NULL);
            }

            emit_effect(cmds_out, num_cmds_out, EFFECT_TASK_DONE, NULL, m->task_num, 0, NULL);
            return (Model) model_new(0, DISPLAY_NONE, NULL);

        case PF_MSG_WIN:
            if (!m->text[0]) {
                fail_runtime("Win text required");
                return (Model) model_new(1, DISPLAY_NONE, NULL);
            }

            emit_effect(cmds_out, num_cmds_out, EFFECT_APPEND_WIN, m->text, 0, 0, NULL);
            return (Model) model_new(0, DISPLAY_NONE, "✓ Win logged");

        case PF_MSG_JOURNAL:
            emit_effect(cmds_out, num_cmds_out, EFFECT_JOURNAL, NULL, 0, 0, NULL);
            return (Model) model_new(0, DISPLAY_NONE, NULL);

        case PF_MSG_POMO:
            /* text holds chunk plan (e.g. "20,4,20,4"); empty → default focus */
            emit_effect(cmds_out, num_cmds_out, EFFECT_POMO, m->text, 0, m->pomo_minutes,
                        m->pomo_plan[0] ? m->pomo_plan : NULL);
            return (Model) model_new(0, DISPLAY_NONE, NULL);

        case PF_MSG_EDIT:
            if (m->edit_todo) {
                emit_effect(cmds_out, num_cmds_out, EFFECT_EDIT_TODO, NULL, 0, 0, NULL);
            } else {
                emit_effect(cmds_out, num_cmds_out, EFFECT_EDIT_LOG, NULL, 0, 0, NULL);
            }

            return (Model) model_new(0, DISPLAY_NONE, NULL);

        case PF_MSG_SEARCH:
            if (!m->text[0]) {
                fail_runtime("Search term required");
                return (Model) model_new(1, DISPLAY_NONE, NULL);
            }

            return (Model) model_with_search(m->text);

        case PF_MSG_STATS:
            return (Model) model_new(0, DISPLAY_STATS, NULL);

        case PF_MSG_REVIEW: {
            PremflowModel *rm = model_new(0, DISPLAY_REVIEW, NULL);
            if (rm) {
                rm->review_full = m->review_full;
            }
            return (Model) rm;
        }

        case PF_MSG_CONFIG_SOUND:
            emit_effect(cmds_out, num_cmds_out, EFFECT_CONFIG_SOUND, NULL, 0, 0, NULL);
            return (Model) model_new(0, DISPLAY_NONE, NULL);

        default:
            return (Model) model_new(1, DISPLAY_NONE, NULL);
    }
}

void pf_view(
    Model model
) {
    PremflowModel *m = (PremflowModel *) model;
    if (!m) {
        return;
    }

    switch (m->display) {
        case DISPLAY_HELP:
            show_help();
            break;
        case DISPLAY_STATS:
            show_stats();
            break;
        case DISPLAY_REVIEW:
            show_review(m->review_full);
            break;
        case DISPLAY_SEARCH:
            if (m->search_term[0]) {
                show_search(m->search_term);
            }
            break;
        case DISPLAY_TASK_LIST: {
            const char *todo_path = data_path(TODO_FILE);
            const int max_to_show = DEFAULT_TASK_LIST_MAX_TO_SHOW;
            list_active_tasks(todo_path, max_to_show);
            break;
        }
        default:
            break;
    }

    if (g_runtime.error[0]) {
        fprintf(stderr, "%s\n", g_runtime.error);
    } else if (m->feedback[0]) {
        printf("%s\n", m->feedback);
    }
}

const char *pf_msg_name(
    Msg msg
) {
    PremflowMsg *m = (PremflowMsg *) msg;
    if (!m) {
        return "NULL";
    }
    switch (m->type) {
        case PF_MSG_HELP:
            return "HELP";
        case PF_MSG_UNKNOWN:
            return "UNKNOWN";
        case PF_MSG_NOTE:
            return "NOTE";
        case PF_MSG_TASK_ADD:
            return "TASK_ADD";
        case PF_MSG_TASK_LIST:
            return "TASK_LIST";
        case PF_MSG_TASK_DONE:
            return "TASK_DONE";
        case PF_MSG_WIN:
            return "WIN";
        case PF_MSG_JOURNAL:
            return "JOURNAL";
        case PF_MSG_POMO:
            return "POMO";
        case PF_MSG_EDIT:
            return "EDIT";
        case PF_MSG_SEARCH:
            return "SEARCH";
        case PF_MSG_STATS:
            return "STATS";
        case PF_MSG_REVIEW:
            return "REVIEW";
        case PF_MSG_CONFIG_SOUND:
            return "CONFIG_SOUND";
        default:
            return "UNKNOWN";
    }
}

void pf_free_model(
    Model m
) {
    free(m);
}

void pf_free_msg(
    Msg m
) {
    free(m);
}

void pf_free_cmd(
    Cmd c
) {
    if (!c) {
        return;
    }
    CmdData *cd = (CmdData *) c;
    if (cd->data) {
        free(cd->data);
    }
    free(cd);
}

static void join_args(
    int start,
    int argc,
    char **argv,
    char *buf,
    size_t len
) {
    buf[0] = '\0';
    for (int i = start; i < argc; i++) {
        if (i > start) {
            strncat(buf, " ", len - strlen(buf) - 1);
        }
        strncat(buf, argv[i], len - strlen(buf) - 1);
    }
}

PremflowMsg *parse_argv(
    int argc,
    char **argv
) {
    PremflowMsg *msg = calloc(1, sizeof(PremflowMsg));
    if (!msg) {
        return NULL;
    }

    if (argc < 2) {
        msg->type = PF_MSG_HELP;
        return msg;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "note") == 0 && argc > 2) {
        msg->type = PF_MSG_NOTE;
        join_args(2, argc, argv, msg->text, sizeof(msg->text));
    } else if (strcmp(cmd, "task") == 0 && argc > 2) {
        if (strcmp(argv[2], "add") == 0 && argc > 3) {
            msg->type = PF_MSG_TASK_ADD;
            join_args(3, argc, argv, msg->text, sizeof(msg->text));
        } else if (strcmp(argv[2], "list") == 0) {
            msg->type = PF_MSG_TASK_LIST;
        } else if (strcmp(argv[2], "done") == 0 && argc > 3) {
            msg->type = PF_MSG_TASK_DONE;
            msg->task_num = atoi(argv[3]);
        } else {
            msg->type = PF_MSG_UNKNOWN;
        }
    } else if (strcmp(cmd, "win") == 0 && argc > 2) {
        msg->type = PF_MSG_WIN;
        join_args(2, argc, argv, msg->text, sizeof(msg->text));
    } else if (strcmp(cmd, "journal") == 0) {
        msg->type = PF_MSG_JOURNAL;
    } else if (strcmp(cmd, "pomo") == 0) {
        msg->type = PF_MSG_POMO;
        msg->text[0] = '\0';
        msg->pomo_plan[0] = '\0';
        msg->pomo_minutes = POMO_DEFAULT_MINUTES;
        /*
         * Usage:
         *   pomo | pomo 25 | pomo 20,4,20,4
         *   pomo 25 ship the review
         *   pomo 20,4 deep work on auth
         *   pomo ship the review          (default plan + context)
         */
        if (argc > 2) {
            pomo_split_args(argc - 2, argv + 2, msg->pomo_plan, sizeof(msg->pomo_plan),
                            msg->text, sizeof(msg->text));
            if (msg->pomo_plan[0]) {
                int probe[POMO_MAX_SEGMENTS];
                int nseg = 0;
                if (pomo_plan_parse(msg->pomo_plan, probe, &nseg, POMO_MAX_SEGMENTS) ==
                    0) {
                    msg->pomo_minutes = probe[0];
                }
            }
        }
    } else if (strcmp(cmd, "edit") == 0) {
        msg->type = PF_MSG_EDIT;
        msg->edit_todo = (argc > 2 && strcmp(argv[2], "todo") == 0) ? 1 : 0;
    } else if (strcmp(cmd, "search") == 0 && argc > 2) {
        msg->type = PF_MSG_SEARCH;
        join_args(2, argc, argv, msg->text, sizeof(msg->text));
    } else if (strcmp(cmd, "stats") == 0) {
        msg->type = PF_MSG_STATS;
    } else if (strcmp(cmd, "review") == 0) {
        msg->type = PF_MSG_REVIEW;
        if (argc > 2) {
            const char *arg = argv[2];
            if (strcmp(arg, "--full") == 0 || strcmp(arg, "full") == 0 ||
                strcmp(arg, "--all") == 0 || strcmp(arg, "all") == 0) {
                msg->review_full = 1;
            }
        }
    } else if (strcmp(cmd, "config") == 0 && argc > 2 &&
               strcmp(argv[2], "sound") == 0) {
        msg->type = PF_MSG_CONFIG_SOUND;
    } else {
        msg->type = PF_MSG_UNKNOWN;
    }

    return msg;
}
