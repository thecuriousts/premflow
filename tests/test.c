#define _POSIX_C_SOURCE 200809L
#include "premflow.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)                                                                     \
    printf("Running " #name "... ");                                                   \
    if (name()) {                                                                      \
        printf("✅ PASSED\n");                                                         \
        tests_passed++;                                                                \
    } else {                                                                           \
        printf("❌ FAILED\n");                                                         \
        tests_failed++;                                                                \
    }

bool test_trim(
    void
) {
    char s1[] = "  hello world  \n";
    assert(strcmp(trim(s1), "hello world") == 0);

    char s2[] = "   \t\n";
    assert(strcmp(trim(s2), "") == 0);

    char s3[] = "no_whitespace";
    assert(strcmp(trim(s3), "no_whitespace") == 0);

    char s4[] = "\t  mixed \t whitespace \n";
    assert(strcmp(trim(s4), "mixed \t whitespace") == 0);

    return true;
}

bool test_data_path(
    void
) {
    char *path = data_path("test.txt");
    assert(path != NULL);
    assert(strstr(path, ".premflow/test.txt") != NULL);
    return true;
}

bool test_append_and_read(
    void
) {
    // Use a temporary file for testing
    char tmpfile[] = "/tmp/premflow_test_XXXXXX";
    int fd = mkstemp(tmpfile);
    close(fd);

    // Test append
    bool ok = append_entry(tmpfile, "[TEST]", "Hello from test");
    assert(ok == true);

    // Read back
    FILE *f = fopen(tmpfile, "r");
    assert(f != NULL);

    char line[256];
    fgets(line, sizeof(line), f);
    fclose(f);

    assert(strstr(line, "[TEST]") != NULL);
    assert(strstr(line, "Hello from test") != NULL);
    assert(ledger_line_matches_contract(line) == true);

    /* Multi-line dump becomes single line */
    ok = append_entry(tmpfile, "[NOTE]", "line one\nline two\r\n  spaced  ");
    assert(ok == true);
    f = fopen(tmpfile, "r");
    assert(f != NULL);
    fgets(line, sizeof(line), f); /* skip first */
    fgets(line, sizeof(line), f);
    fclose(f);
    assert(strstr(line, "\nline") == NULL);
    assert(strstr(line, "line one line two spaced") != NULL);
    assert(ledger_line_matches_contract(line) == true);

    unlink(tmpfile); // cleanup
    return true;
}

bool test_ledger_contract_helpers(
    void
) {
    char body[MAX_LINE];
    char done[MAX_LINE];

    assert(ledger_sanitize_body("  hello\nworld\t ", body, sizeof(body)) == true);
    assert(strcmp(body, "hello world") == 0);
    assert(ledger_sanitize_body("   \n\t", body, sizeof(body)) == false);

    /* Nested historical TODO shape → plain title */
    ledger_clean_done_body("[2026-04-11 23:18] [TODO] Test task to complete\n", done,
                           sizeof(done));
    assert(strcmp(done, "Test task to complete") == 0);

    ledger_clean_done_body("[2026-04-11 23:18] [DONE] [2026-04-11 23:18] [TODO] Nested",
                           done, sizeof(done));
    assert(strcmp(done, "Nested") == 0);

    assert(ledger_line_matches_contract(
               "[2026-07-15 12:00] [NOTE] ship it\n") == true);
    assert(ledger_line_matches_contract("not a ledger line") == false);
    assert(ledger_line_matches_contract("[2026-07-15 12:00] [NOTE]\n") == false);

    return true;
}

bool test_complete_task(
    void
) {
    char tmpfile[] = "/tmp/premflow_todo_XXXXXX";
    int fd = mkstemp(tmpfile);
    close(fd);

    // Create sample todo file
    FILE *f = fopen(tmpfile, "w");
    fprintf(f, "[TODO] First task\n");
    fprintf(f, "[TODO] Second task\n");
    fprintf(f, "[TODO] Third task\n");
    fclose(f);

    // Complete task #2
    bool ok = complete_task(tmpfile, 2);
    assert(ok == true);

    // Verify remaining tasks
    f = fopen(tmpfile, "r");
    char line1[128], line2[128];
    fgets(line1, sizeof(line1), f);
    fgets(line2, sizeof(line2), f);
    fclose(f);

    assert(strstr(line1, "First task") != NULL);
    assert(strstr(line2, "Third task") != NULL);

    unlink(tmpfile);
    return true;
}

bool test_config_template(
    void
) {
    char tmpfile[] = "/tmp/premflow_config_XXXXXX";
    int fd = mkstemp(tmpfile);
    close(fd);

    create_config_template(tmpfile);

    FILE *f = fopen(tmpfile, "r");
    assert(f != NULL);

    char line[256];
    bool has_player = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "PLAYER=")) {
            has_player = true;
        }
    }
    fclose(f);

    assert(has_player == true);
    unlink(tmpfile);
    return true;
}

bool test_journal_path(
    void
) {
    char *path = journal_path();
    assert(path != NULL);
    assert(strstr(path, "journal-") != NULL);
    assert(strstr(path, ".txt") != NULL);
    return true;
}

bool test_ensure_journal(
    void
) {
    /* Drive real ensure_journal against a temp HOME */
    char td[] = "/tmp/premflow_jhome_XXXXXX";
    assert(mkdtemp(td) != NULL);
    char *old_home = getenv("HOME");
    setenv("HOME", td, 1);

    char path[512];
    int created = 0;
    assert(ensure_journal(path, sizeof(path), &created) == true);
    assert(created == 1);
    assert(strstr(path, "journal-") != NULL);
    assert(strstr(path, ".premflow/journal/") != NULL);

    FILE *f = fopen(path, "r");
    assert(f != NULL);
    char buf[512];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    fclose(f);
    assert(strstr(buf, "Grateful for") != NULL);
    assert(strstr(buf, "Tomorrow's intention") != NULL);

    /* Second call: not created again */
    created = -1;
    assert(ensure_journal(path, sizeof(path), &created) == true);
    assert(created == 0);

    /* cleanup */
    unlink(path);
    char jdir[600], pdir[600];
    snprintf(jdir, sizeof(jdir), "%s/.premflow/journal", td);
    snprintf(pdir, sizeof(pdir), "%s/.premflow", td);
    rmdir(jdir);
    rmdir(pdir);
    rmdir(td);
    if (old_home) {
        setenv("HOME", old_home, 1);
    } else {
        unsetenv("HOME");
    }
    return true;
}

bool test_pomo_plan_parse(
    void
) {
    int minutes[POMO_MAX_SEGMENTS];
    int count = 0;

    /* Default / empty → single 25 focus */
    assert(pomo_plan_parse(NULL, minutes, &count, POMO_MAX_SEGMENTS) == 0);
    assert(count == 1 && minutes[0] == POMO_DEFAULT_MINUTES);

    assert(pomo_plan_parse("", minutes, &count, POMO_MAX_SEGMENTS) == 0);
    assert(count == 1 && minutes[0] == 25);

    /* Single integer */
    assert(pomo_plan_parse("50", minutes, &count, POMO_MAX_SEGMENTS) == 0);
    assert(count == 1 && minutes[0] == 50);

    /* Classic multi-chunk: focus 20, break 4, focus 20, break 4 */
    assert(pomo_plan_parse("20,4,20,4", minutes, &count, POMO_MAX_SEGMENTS) == 0);
    assert(count == 4);
    assert(minutes[0] == 20 && minutes[1] == 4 && minutes[2] == 20 && minutes[3] == 4);

    /* Whitespace around commas */
    assert(pomo_plan_parse(" 15 , 5 , 15 ", minutes, &count, POMO_MAX_SEGMENTS) == 0);
    assert(count == 3 && minutes[0] == 15 && minutes[1] == 5 && minutes[2] == 15);

    /* Invalid plans */
    assert(pomo_plan_parse("0", minutes, &count, POMO_MAX_SEGMENTS) == -1);
    assert(pomo_plan_parse("-5", minutes, &count, POMO_MAX_SEGMENTS) == -1);
    assert(pomo_plan_parse("abc", minutes, &count, POMO_MAX_SEGMENTS) == -1);
    assert(pomo_plan_parse("20,", minutes, &count, POMO_MAX_SEGMENTS) == -1);
    assert(pomo_plan_parse("20,,4", minutes, &count, POMO_MAX_SEGMENTS) == -1);
    assert(pomo_plan_parse("20a", minutes, &count, POMO_MAX_SEGMENTS) == -1);

    return true;
}

bool test_pomo_tick_and_pause(
    void
) {
    int plan[] = {1, 1}; /* 1 min focus, 1 min break — tick in seconds */
    PomoSession s;
    pomo_session_init(&s, plan, 2);
    assert(s.running == 1);
    assert(s.paused == 0);
    assert(s.remaining_seconds == 60);
    assert(pomo_session_phase(&s) == POMO_PHASE_FOCUS);

    /* Tick decreases remaining only when not paused */
    assert(pomo_session_tick(&s) == POMO_EVT_TICK);
    assert(s.remaining_seconds == 59);

    pomo_session_apply(&s, POMO_CTRL_PAUSE_TOGGLE);
    assert(s.paused == 1);
    int frozen = s.remaining_seconds;
    assert(pomo_session_tick(&s) == POMO_EVT_NONE);
    assert(s.remaining_seconds == frozen);

    pomo_session_apply(&s, POMO_CTRL_PAUSE_TOGGLE);
    assert(s.paused == 0);
    assert(pomo_session_tick(&s) == POMO_EVT_TICK);
    assert(s.remaining_seconds == frozen - 1);

    return true;
}

bool test_pomo_restart_and_reset(
    void
) {
    int plan[] = {2, 1, 2}; /* 2m focus, 1m break, 2m focus */
    PomoSession s;
    pomo_session_init(&s, plan, 3);

    /* Burn some time on segment 0 */
    for (int i = 0; i < 30; i++) {
        pomo_session_tick(&s);
    }
    assert(s.remaining_seconds == 2 * 60 - 30);
    assert(s.current_index == 0);

    /* Restart restores current segment full duration without advancing index */
    pomo_session_apply(&s, POMO_CTRL_RESTART);
    assert(s.current_index == 0);
    assert(s.remaining_seconds == 120);
    assert(s.paused == 0);

    /* Advance to segment 1 by completing remaining seconds */
    s.remaining_seconds = 1;
    assert(pomo_session_tick(&s) == POMO_EVT_SEGMENT_COMPLETE);
    assert(s.current_index == 1);
    assert(pomo_session_phase(&s) == POMO_PHASE_BREAK);
    assert(s.remaining_seconds == 60);
    assert(s.last_completed_phase == POMO_PHASE_FOCUS);

    /* Tick into break, then reset → segment 0 full duration */
    pomo_session_tick(&s);
    assert(s.remaining_seconds == 59);
    pomo_session_apply(&s, POMO_CTRL_RESET);
    assert(s.current_index == 0);
    assert(s.remaining_seconds == 120);
    assert(s.paused == 0);
    assert(pomo_session_phase(&s) == POMO_PHASE_FOCUS);

    return true;
}

bool test_pomo_segment_advance_and_plan_complete(
    void
) {
    /* Use 1-second segments by initializing then overriding remaining_seconds.
     * Plan minutes stay positive for init; we drive via remaining_seconds. */
    int plan[] = {1, 1, 1, 1}; /* 20,4,20,4 shape — four segments */
    PomoSession s;
    pomo_session_init(&s, plan, 4);

    assert(pomo_session_phase(&s) == POMO_PHASE_FOCUS);
    s.remaining_seconds = 1;
    assert(pomo_session_tick(&s) == POMO_EVT_SEGMENT_COMPLETE);
    assert(s.last_completed_phase == POMO_PHASE_FOCUS);
    assert(s.current_index == 1);
    assert(pomo_session_phase(&s) == POMO_PHASE_BREAK);

    s.remaining_seconds = 1;
    assert(pomo_session_tick(&s) == POMO_EVT_SEGMENT_COMPLETE);
    assert(s.last_completed_phase == POMO_PHASE_BREAK);
    assert(s.current_index == 2);
    assert(pomo_session_phase(&s) == POMO_PHASE_FOCUS);

    s.remaining_seconds = 1;
    assert(pomo_session_tick(&s) == POMO_EVT_SEGMENT_COMPLETE);
    assert(s.current_index == 3);
    assert(pomo_session_phase(&s) == POMO_PHASE_BREAK);

    s.remaining_seconds = 1;
    assert(pomo_session_tick(&s) == POMO_EVT_PLAN_COMPLETE);
    assert(s.last_completed_phase == POMO_PHASE_BREAK);
    assert(s.running == 0);

    /* Further ticks do nothing */
    assert(pomo_session_tick(&s) == POMO_EVT_NONE);

    /* Parse of real 20,4,20,4 lengths */
    int m[4];
    int n = 0;
    assert(pomo_plan_parse("20,4,20,4", m, &n, 4) == 0);
    pomo_session_init(&s, m, n);
    assert(s.segment_count == 4);
    assert(s.segment_minutes[0] == 20 && s.segment_minutes[1] == 4);
    assert(s.segment_minutes[2] == 20 && s.segment_minutes[3] == 4);
    assert(pomo_session_segment_seconds(&s) == 20 * 60);

    return true;
}

bool test_pomodoro_logic(
    void
) {
    /* Aggregate smoke: parse → init → pause/resume → restart → reset → complete */
    int m[POMO_MAX_SEGMENTS];
    int n = 0;
    assert(pomo_plan_parse("20,4,20,4", m, &n, POMO_MAX_SEGMENTS) == 0);
    assert(n == 4);

    PomoSession s;
    pomo_session_init(&s, m, n);
    assert(s.remaining_seconds == 20 * 60);

    pomo_session_apply(&s, POMO_CTRL_PAUSE_TOGGLE);
    int rem = s.remaining_seconds;
    assert(pomo_session_tick(&s) == POMO_EVT_NONE);
    assert(s.remaining_seconds == rem);

    pomo_session_apply(&s, POMO_CTRL_PAUSE_TOGGLE);
    s.remaining_seconds = 10;
    for (int i = 0; i < 5; i++) {
        pomo_session_tick(&s);
    }
    assert(s.remaining_seconds == 5);
    pomo_session_apply(&s, POMO_CTRL_RESTART);
    assert(s.remaining_seconds == 20 * 60);

    s.remaining_seconds = 1;
    assert(pomo_session_tick(&s) == POMO_EVT_SEGMENT_COMPLETE);
    pomo_session_apply(&s, POMO_CTRL_RESET);
    assert(s.current_index == 0 && s.remaining_seconds == 20 * 60);

    return true;
}

bool test_pomo_context_and_split(
    void
) {
    char plan[128];
    char ctx[MAX_LINE];
    char body[MAX_LINE];

    /* No args → empty plan + empty context */
    pomo_split_args(0, NULL, plan, sizeof(plan), ctx, sizeof(ctx));
    assert(plan[0] == '\0' && ctx[0] == '\0');

    /* Plan only */
    char *a1[] = {"25"};
    pomo_split_args(1, a1, plan, sizeof(plan), ctx, sizeof(ctx));
    assert(strcmp(plan, "25") == 0);
    assert(ctx[0] == '\0');

    /* Chunk plan + multi-word context */
    char *a2[] = {"20,4,20,4", "ship", "review", "PR"};
    pomo_split_args(4, a2, plan, sizeof(plan), ctx, sizeof(ctx));
    assert(strcmp(plan, "20,4,20,4") == 0);
    assert(strcmp(ctx, "ship review PR") == 0);

    /* Context only (no plan token) → default plan empty, full context */
    char *a3[] = {"deep", "work", "on", "auth"};
    pomo_split_args(4, a3, plan, sizeof(plan), ctx, sizeof(ctx));
    assert(plan[0] == '\0');
    assert(strcmp(ctx, "deep work on auth") == 0);

    /* Single plan + single context word */
    char *a4[] = {"50", "writing"};
    pomo_split_args(2, a4, plan, sizeof(plan), ctx, sizeof(ctx));
    assert(strcmp(plan, "50") == 0);
    assert(strcmp(ctx, "writing") == 0);

    /* Log body formatting */
    pomo_format_log_body(NULL, body, sizeof(body));
    assert(strcmp(body, "pomodoro session") == 0);
    pomo_format_log_body("", body, sizeof(body));
    assert(strcmp(body, "pomodoro session") == 0);
    pomo_format_log_body("ship review PR", body, sizeof(body));
    assert(strcmp(body, "ship review PR") == 0);

    return true;
}

bool test_journal_creation(
    void
) {
    char tmp_journal[] = "/tmp/premflow_journal_XXXXXX";
    int fd = mkstemp(tmp_journal);
    close(fd);

    // Simulate journal creation logic
    FILE *f = fopen(tmp_journal, "w");
    assert(f != NULL);

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char date[64];
    strftime(date, sizeof(date), "%A, %B %d, %Y", tm);

    fprintf(f,
            "# 🌟 Daily Journal — %s\n\n"
            "🙏 Grateful for:\n1. \n2. \n3. \n\n"
            "📚 Learned today:\n\n"
            "🚀 Tomorrow's intention:\n\n"
            "💡 Today's win:\n\n",
            date);
    fclose(f);

    // Verify content
    f = fopen(tmp_journal, "r");
    char line[256];
    bool has_grateful = false;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "Grateful for")) {
            has_grateful = true;
        }
    }
    fclose(f);

    assert(has_grateful == true);
    unlink(tmp_journal);
    return true;
}

int main(
    void
) {
    printf("=== premflow Comprehensive Tests (with file I/O) ===\n\n");

    TEST(test_trim);
    TEST(test_data_path);
    TEST(test_append_and_read);
    TEST(test_ledger_contract_helpers);
    TEST(test_complete_task);
    TEST(test_config_template);
    TEST(test_journal_path);
    TEST(test_ensure_journal);
    TEST(test_pomo_plan_parse);
    TEST(test_pomo_tick_and_pause);
    TEST(test_pomo_restart_and_reset);
    TEST(test_pomo_segment_advance_and_plan_complete);
    TEST(test_pomodoro_logic);
    TEST(test_pomo_context_and_split);
    TEST(test_journal_creation);

    printf("\n=====================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    if (tests_failed == 0) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("⚠️  Some tests failed.\n");
        return 1;
    }
}
