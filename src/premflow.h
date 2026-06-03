#ifndef PREMFLOW_H
#define PREMFLOW_H

#include <stdbool.h>
#include <stdio.h>

#define MAX_LINE 1024
#define DATA_DIR ".premflow"
#define LOG_FILE "log.txt"
#define TODO_FILE "todo.txt"
#define CONFIG_FILE "config.txt"

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
bool append_entry(const char *filepath, const char *prefix, const char *text);
void play_sound(const char *command);
void open_editor(const char *filepath);

void start_pomodoro(int minutes);
void list_active_tasks(const char *filepath, int max_to_show); /* max_to_show <=0 means all */
bool complete_task(const char *filepath, int task_num);

// UI
void show_help(void);
void show_stats(void);
void show_review(int full);
void show_search(const char *term);

#endif
