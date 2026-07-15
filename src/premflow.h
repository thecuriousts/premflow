#ifndef PREMFLOW_H
#define PREMFLOW_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define MAX_LINE 1024
#define DATA_DIR ".premflow"
#define LOG_FILE "log.txt"
#define TODO_FILE "todo.txt"
#define CONFIG_FILE "config.txt"

/* list_active_tasks(filepath, max_to_show): <=0 applies no listing cap */
#define DEFAULT_TASK_LIST_MAX_TO_SHOW 0
#define REVIEW_TASK_LIST_MAX_TO_SHOW 8

typedef struct {
    char player[256];
    char pomo_start[512];
    char pomo_complete[512];
    char task_complete[512];
} SoundConfig;

extern SoundConfig sounds;

// String utilities
char *ltrim(char *s);
void rtrim(char *s);
char *trim(char *s);

// Path helpers
char *data_path(const char *filename);
char *journal_path(void);

// Config
void create_config_template(const char *path);
void read_config(void);

// Core logic (return bool for success/failure)
bool ensure_dirs(void);

/* Collapse CR/LF/TAB to spaces, trim, single-line body for ledger clarity.
 * Returns false if result would be empty. Always null-terminates out when out_sz>0. */
bool ledger_sanitize_body(const char *in, char *out, size_t out_sz);

/* Strip leading [ts] and [TODO]/[TYPE] tags from a todo line for DONE body. */
void ledger_clean_done_body(const char *task_line, char *out, size_t out_sz);

/* True if line matches [YYYY-MM-DD HH:MM] [TYPE] non-empty body (TYPE = A-Z+). */
bool ledger_line_matches_contract(const char *line);

bool append_entry(const char *filepath, const char *prefix, const char *text);
void play_sound(const char *command);
void open_editor(const char *filepath);

/* Ensure today's journal exists (create template if missing). Writes absolute path
 * to path_out. Returns true on success. Does not open an editor. */
bool ensure_journal(char *path_out, size_t path_out_sz, int *created_out);

/* Multi-segment pomodoro session engine (pure; unit-testable) */
#define POMO_MAX_SEGMENTS 32
#define POMO_DEFAULT_MINUTES 25

typedef enum {
    POMO_PHASE_FOCUS = 0,
    POMO_PHASE_BREAK = 1
} PomoPhase;

typedef enum {
    POMO_CTRL_NONE = 0,
    POMO_CTRL_PAUSE_TOGGLE,
    POMO_CTRL_RESTART, /* current segment → full planned duration */
    POMO_CTRL_RESET,   /* plan → segment 0 at full duration */
    POMO_CTRL_QUIT
} PomoControl;

typedef enum {
    POMO_EVT_NONE = 0,
    POMO_EVT_TICK,
    POMO_EVT_SEGMENT_COMPLETE, /* advanced to next segment; completed was focus/break */
    POMO_EVT_PLAN_COMPLETE,    /* last segment finished */
    POMO_EVT_QUIT
} PomoEvent;

typedef struct {
    int segment_minutes[POMO_MAX_SEGMENTS];
    int segment_count;
    int current_index;
    int remaining_seconds;
    int paused;
    int running; /* 0 when plan complete or quit */
    PomoPhase last_completed_phase; /* valid after SEGMENT_COMPLETE / PLAN_COMPLETE */
} PomoSession;

/* Parse "20,4,20,4" or "25". NULL/empty → single POMO_DEFAULT_MINUTES.
 * Returns 0 on success, -1 on invalid (writes nothing to outs on failure). */
int pomo_plan_parse(const char *spec, int *minutes_out, int *count_out, int max_count);

void pomo_session_init(PomoSession *s, const int *minutes, int count);
PomoPhase pomo_session_phase(const PomoSession *s);
int pomo_session_segment_seconds(const PomoSession *s);
void pomo_session_apply(PomoSession *s, PomoControl ctrl);
/* Advance one second when not paused. On segment end: log phase in last_completed_phase. */
PomoEvent pomo_session_tick(PomoSession *s);

/* Log body for [POMO] entries: context if non-empty, else "pomodoro session". */
void pomo_format_log_body(const char *context, char *out, size_t out_sz);

/* Split args after "pomo": if first token is a valid plan, it becomes the plan and
 * remaining tokens are context; otherwise all tokens are context (default plan).
 * plan_out may be empty (caller uses default). Always null-terminates outs. */
void pomo_split_args(int argc, char **argv, char *plan_out, size_t plan_sz,
                     char *context_out, size_t context_sz);

/* Live interactive session (TTY keys when available).
 * plan_spec NULL/"" → 25 min focus. context NULL/"" → generic log label. */
void start_pomodoro(const char *plan_spec, const char *context);
void list_active_tasks(const char *filepath, int max_to_show);
bool complete_task(const char *filepath, int task_num);

// UI
void show_help(void);
void show_stats(void);
void show_review(int full);
void show_search(const char *term);

#endif
