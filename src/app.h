#ifndef APP_H
#define APP_H

#include "premflow.h"
#include <elomaxz.h>
#include <stddef.h>

typedef enum {
    PF_MSG_HELP,
    PF_MSG_UNKNOWN,
    PF_MSG_NOTE,
    PF_MSG_TASK_ADD,
    PF_MSG_TASK_LIST,
    PF_MSG_TASK_DONE,
    PF_MSG_WIN,
    PF_MSG_JOURNAL,
    PF_MSG_POMO,
    PF_MSG_EDIT,
    PF_MSG_SEARCH,
    PF_MSG_STATS,
    PF_MSG_REVIEW,
    PF_MSG_CONFIG_SOUND
} PremflowMsgType;

typedef struct {
    PremflowMsgType type;
    char text[MAX_LINE];
    int task_num;
    int pomo_minutes;
    int edit_todo; /* 1 = todo file, 0 = log file */
} PremflowMsg;

typedef enum {
    DISPLAY_NONE,
    DISPLAY_HELP,
    DISPLAY_STATS,
    DISPLAY_REVIEW,
    DISPLAY_SEARCH,
    DISPLAY_TASK_LIST
} DisplayKind;

typedef struct {
    int exit_code;
    DisplayKind display;
    char feedback[MAX_LINE];
    char search_term[MAX_LINE];
} PremflowModel;

typedef enum {
    EFFECT_APPEND_NOTE,
    EFFECT_APPEND_TODO,
    EFFECT_APPEND_WIN,
    EFFECT_TASK_DONE,
    EFFECT_JOURNAL,
    EFFECT_POMO,
    EFFECT_EDIT_LOG,
    EFFECT_EDIT_TODO,
    EFFECT_CONFIG_SOUND
} EffectKind;

typedef struct {
    EffectKind kind;
    char text[MAX_LINE];
    int task_num;
    int pomo_minutes;
} EffectPayload;

typedef struct {
    int exit_code;
    char error[MAX_LINE];
} PremflowRuntime;

Model pf_init(void);
Model pf_update(Model current, Msg msg, Cmd *cmds_out, size_t *num_cmds_out);
void pf_view(Model model);
const char *pf_msg_name(Msg msg);
void pf_free_model(Model m);
void pf_free_msg(Msg m);
void pf_free_cmd(Cmd c);
void pf_handle_cmd(Cmd cmd, Msg *result_msg);

PremflowMsg *parse_argv(int argc, char **argv);
void premflow_runtime_init(PremflowRuntime *rt);
PremflowRuntime *premflow_runtime_get(void);

#endif
