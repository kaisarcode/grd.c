/**
 * test.c - libgrd public API tests.
 * Summary: Tests each public libgrd function through one CTest case.
 *
 * Author:  KaisarCode
 * Website: https://kaisarcode.com
 * License: https://www.gnu.org/licenses/gpl-3.0.html
 */

#include "grd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <process.h>
#define getpid _getpid
#else
#include <signal.h>
#include <unistd.h>
#endif

static int signal_count = 0;
static kc_grd_box_t *signal_ctx_seen = NULL;

/**
 * Records one signal callback invocation.
 * @param ctx Context supplied by the library.
 * @return None.
 */
static void count_signal(kc_grd_box_t *ctx) {
    if (ctx) { signal_count++; signal_ctx_seen = ctx; }
}

static int signal_count_b = 0;

/**
 * Records one replacement signal callback invocation.
 * @param ctx Context supplied by the library.
 * @return None.
 */
static void count_signal_b(kc_grd_box_t *ctx) {
    if (ctx) { signal_count_b++; }
}

/**
 * Verifies one integer result.
 * @param name Check description.
 * @param expected Expected value.
 * @param actual Actual value.
 * @return 0 on success, 1 on failure.
 */
static int expect_int(const char *name, int expected, int actual) {
    if (expected != actual) {
        printf("\033[31m[FAIL]\033[0m %s: expected %d, got %d\n", name, expected, actual);
        return 1;
    }
    printf("\033[32m[PASS]\033[0m %s\n", name);
    return 0;
}

/**
 * Verifies one boolean condition.
 * @param name Check description.
 * @param condition Non-zero when the check passed.
 * @return 0 on success, 1 on failure.
 */
static int expect_true(const char *name, int condition) {
    if (!condition) {
        printf("\033[31m[FAIL]\033[0m %s\n", name);
        return 1;
    }
    printf("\033[32m[PASS]\033[0m %s\n", name);
    return 0;
}

/**
 * Tests kc_grd_version.
 * @return 0 on success, 1 on failure.
 */
static int case_version(void) {
    return expect_true("version returns non-zero", kc_grd_version() != 0U);
}

/**
 * Tests kc_grd_options_default.
 * @return 0 on success, 1 on failure.
 */
static int case_options_default(void) {
    kc_grd_options_t opts;
    opts = kc_grd_options_default();
    return expect_true("default options zeroed", opts.width == 0 && opts.height == 0);
}

/**
 * Tests kc_grd_options_load_env.
 * @return 0 on success, 1 on failure.
 */
static int case_options_load_env(void) {
    kc_grd_options_t opts = {0};
    kc_grd_options_load_env(&opts);
    kc_grd_options_load_env(NULL);
    return expect_true("load_env does not crash", 1);
}

/**
 * Tests kc_grd_options_free.
 * @return 0 on success, 1 on failure.
 */
static int case_options_free(void) {
    kc_grd_options_t opts = {0};
    kc_grd_options_free(&opts);
    kc_grd_options_free(NULL);
    return expect_true("options_free does not crash", 1);
}

/**
 * Tests kc_grd_box_new.
 * @return 0 on success, 1 on failure.
 */
static int case_box_new(void) {
    kc_grd_box_t *b;
    b = kc_grd_box_new();
    if (!b) return 1;
    expect_true("new box border=1", b->border == 1);
    expect_true("new box padding=1", b->padding == 1);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_box_free.
 * @return 0 on success, 1 on failure.
 */
static int case_box_free(void) {
    kc_grd_box_free(NULL);
    return expect_true("free(NULL) does not crash", 1);
}

/**
 * Tests kc_grd_split_set.
 * @return 0 on success, 1 on failure.
 */
static int case_split_set(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_split_t *s;
    s = kc_grd_split_set(NULL, KC_GRD_ROW);
    expect_true("split_set(NULL) returns NULL", s == NULL);
    s = kc_grd_split_set(b, KC_GRD_ROW);
    expect_true("split_set creates split", s != NULL);
    expect_true("split kind is ROW", s->kind == KC_GRD_ROW);
    expect_true("box has split", b->split == s);
    s = kc_grd_split_set(b, KC_GRD_COL);
    expect_true("split_set replaces", s->kind == KC_GRD_COL);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_split_add.
 * @return 0 on success, 1 on failure.
 */
static int case_split_add(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_box_t *c2 = kc_grd_box_new();
    expect_int("split_add(NULL) returns -1", -1, kc_grd_split_add(NULL, c1, 1.0f));
    expect_int("split_add child NULL returns -1", -1, kc_grd_split_add(s, NULL, 1.0f));
    expect_int("split_add c1 returns 0", 0, kc_grd_split_add(s, c1, 1.0f));
    expect_int("split_add c2 returns 0", 0, kc_grd_split_add(s, c2, 2.0f));
    expect_int("split count is 2", 2, s->count);
    expect_true("c1 parent set", c1->parent == b);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_split_weight.
 * @return 0 on success, 1 on failure.
 */
static int case_split_weight(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_split_add(s, c1, 1.0f);
    expect_int("split_weight NULL returns -1", -1, kc_grd_split_weight(NULL, 0, 1.0f));
    expect_int("split_weight invalid idx returns -1", -1, kc_grd_split_weight(s, 5, 1.0f));
    expect_int("split_weight valid returns 0", 0, kc_grd_split_weight(s, 0, 3.0f));
    expect_true("weight updated", s->weights[0] == 3.0f);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_box_bounds.
 * @return 0 on success, 1 on failure.
 */
static int case_box_bounds(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_box_bounds(NULL, 0, 0, 100, 100);
    kc_grd_box_bounds(b, 10, 20, 200, 300);
    expect_int("box x set", 10, b->x);
    expect_int("box y set", 20, b->y);
    expect_int("box w set", 200, b->w);
    expect_int("box h set", 300, b->h);
    expect_true("inner_x computed", b->inner_x >= b->x);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_box_layout.
 * @return 0 on success, 1 on failure.
 */
static int case_box_layout(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_box_layout(NULL);
    kc_grd_box_bounds(b, 0, 0, 100, 100);
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_box_t *c2 = kc_grd_box_new();
    kc_grd_split_add(s, c1, 1.0f);
    kc_grd_split_add(s, c2, 1.0f);
    kc_grd_box_layout(b);
    expect_true("c1 has width", c1->w > 0);
    expect_true("c2 has width", c2->w > 0);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_split_gap.
 * @return 0 on success, 1 on failure.
 */
static int case_split_gap(void) {
    kc_grd_split_gap(NULL, 5, 10);
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_split_gap(s, 5, 10);
    expect_int("gap set", 5, s->gap);
    expect_int("min_px set", 10, s->min_px);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_gap_hit.
 * @return 0 on success, 1 on failure.
 */
static int case_gap_hit(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_gap_t gap;
    expect_int("gap_hit(NULL) returns 0", 0, kc_grd_gap_hit(NULL, 0, 0, &gap));
    expect_int("gap_hit no split returns 0", 0, kc_grd_gap_hit(b, 0, 0, &gap));
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_box_t *c2 = kc_grd_box_new();
    kc_grd_split_add(s, c1, 1.0f);
    kc_grd_split_add(s, c2, 1.0f);
    kc_grd_split_gap(s, 4, 4);
    kc_grd_box_bounds(b, 0, 0, 200, 100);
    kc_grd_box_layout(b);
    int hit = kc_grd_gap_hit(b, c1->x + c1->w + 1, 50, &gap);
    expect_true("gap_hit finds gap", hit == 1);
    expect_true("gap hit has split", gap.split == s);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_drag_begin, update, and end.
 * @return 0 on success, 1 on failure.
 */
static int case_drag(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_box_t *c2 = kc_grd_box_new();
    kc_grd_split_add(s, c1, 1.0f);
    kc_grd_split_add(s, c2, 1.0f);
    kc_grd_split_gap(s, 4, 4);
    kc_grd_box_bounds(b, 0, 0, 200, 100);
    kc_grd_box_layout(b);
    kc_grd_gap_t gap;
    kc_grd_gap_hit(b, c1->x + c1->w + 1, 50, &gap);
    expect_int("drag_begin returns 0", 0, kc_grd_drag_begin(&gap, gap.x, gap.y));
    expect_true("drag_on set", s->drag_on == 1);
    expect_int("drag_update returns 0", 0, kc_grd_drag_update(s, gap.x + 10, gap.y));
    kc_grd_drag_end(s);
    expect_int("drag_on cleared", 0, s->drag_on);
    expect_int("drag_begin NULL returns -1", -1, kc_grd_drag_begin(NULL, 0, 0));
    expect_int("drag_update no drag returns -1", -1, kc_grd_drag_update(s, 0, 0));
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_box_close.
 * @return 0 on success, 1 on failure.
 */
static int case_box_close(void) {
    kc_grd_box_t *root = kc_grd_box_new();
    expect_int("box_close no parent returns -1", -1, kc_grd_box_close(NULL));
    expect_int("box_close no parent returns -1", -1, kc_grd_box_close(root));
    kc_grd_split_t *s = kc_grd_split_set(root, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_box_t *c2 = kc_grd_box_new();
    kc_grd_box_t *c3 = kc_grd_box_new();
    kc_grd_split_add(s, c1, 1.0f);
    kc_grd_split_add(s, c2, 1.0f);
    kc_grd_split_add(s, c3, 1.0f);
    expect_int("close c1 returns 0", 0, kc_grd_box_close(c1));
    expect_int("remaining count is 2", 2, s->count);
    kc_grd_box_free(root);
    return 0;
}

/**
 * Tests kc_grd_split_at.
 * @return 0 on success, 1 on failure.
 */
static int case_split_at(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_split_t *s = kc_grd_split_set(b, KC_GRD_ROW);
    kc_grd_box_t *c1 = kc_grd_box_new();
    kc_grd_split_add(s, c1, 1.0f);
    expect_true("split_at(NULL) returns NULL", kc_grd_split_at(NULL, 0) == NULL);
    expect_true("split_at invalid returns NULL", kc_grd_split_at(s, 5) == NULL);
    expect_true("split_at(0) returns c1", kc_grd_split_at(s, 0) == c1);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_stop.
 * @return 0 on success, 1 on failure.
 */
static int case_stop(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_stop(NULL);
    expect_true("stop(NULL) no crash", 1);
    expect_true("stop not requested", b->stop_requested == 0);
    kc_grd_stop(b);
    expect_true("stop requested", b->stop_requested == 1);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_on_signal.
 * @return 0 on success, 1 on failure.
 */
static int case_on_signal(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    signal_count = 0;
    signal_count_b = 0;
    expect_int("on_signal NULL returns -1", -1, kc_grd_on_signal(NULL, 1, count_signal));
    expect_int("remove missing returns 0", 0, kc_grd_on_signal(b, 1, NULL));
    expect_int("register returns 0", 0, kc_grd_on_signal(b, 1, count_signal));
    expect_int("raise returns 0", 0, kc_grd_raise_signal(b, 1));
    expect_int("handler invoked", 1, signal_count);
    expect_int("replace returns 0", 0, kc_grd_on_signal(b, 1, count_signal_b));
    signal_count = 0; signal_count_b = 0;
    kc_grd_raise_signal(b, 1);
    expect_int("old not invoked", 0, signal_count);
    expect_int("replacement invoked", 1, signal_count_b);
    expect_int("remove returns 0", 0, kc_grd_on_signal(b, 1, NULL));
    expect_int("raise removed returns -1", -1, kc_grd_raise_signal(b, 1));
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_raise_signal.
 * @return 0 on success, 1 on failure.
 */
static int case_raise_signal(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    signal_count = 0;
    signal_ctx_seen = NULL;
    expect_int("raise_signal NULL returns -1", -1, kc_grd_raise_signal(NULL, 1));
    expect_int("raise unhandled returns -1", -1, kc_grd_raise_signal(b, 1));
    kc_grd_on_signal(b, 1, count_signal);
    expect_int("raise handled returns 0", 0, kc_grd_raise_signal(b, 1));
    expect_true("context matches", signal_ctx_seen == b);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_listen_signals.
 * @return 0 on success, 1 on failure.
 */
static int case_listen_signals(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    signal_count = 0;
    signal_ctx_seen = NULL;
    expect_int("listen_signals NULL returns -1", -1, kc_grd_listen_signals(NULL));
    kc_grd_on_signal(b, 44, count_signal);
    expect_int("listen_signals returns 0", 0, kc_grd_listen_signals(b));
    kc_grd_signal_listener(44);
    expect_int("listener dispatched", 1, signal_count);
    expect_true("correct context", signal_ctx_seen == b);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_listen_signal.
 * @return 0 on success, 1 on failure.
 */
static int case_listen_signal(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    expect_int("listen_signal NULL returns -1", -1, kc_grd_listen_signal(NULL, 1));
#ifdef _WIN32
    expect_int("listen_signal returns 0", 0, kc_grd_listen_signal(b, 2));
#else
    expect_int("listen_signal returns 0", 0, kc_grd_listen_signal(b, SIGUSR1));
#endif
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests kc_grd_signal_listener.
 * @return 0 on success, 1 on failure.
 */
static int case_signal_listener(void) {
    kc_grd_box_t *b = kc_grd_box_new();
    signal_count = 0;
    signal_ctx_seen = NULL;
    kc_grd_on_signal(b, 55, count_signal);
    kc_grd_listen_signals(b);
    kc_grd_signal_listener(55);
    expect_int("signal_listener invokes", 1, signal_count);
    expect_true("correct context", signal_ctx_seen == b);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Tests two contexts coexist.
 * @return 0 on success, 1 on failure.
 */
static int case_multictx(void) {
    kc_grd_box_t *a = kc_grd_box_new();
    kc_grd_box_t *b = kc_grd_box_new();
    kc_grd_stop(a); kc_grd_stop(b);
    kc_grd_stop(a);
    expect_true("a stop_requested", a->stop_requested);
    expect_true("b stop_requested", b->stop_requested);
    kc_grd_box_free(a);
    kc_grd_box_free(b);
    return 0;
}

/**
 * Runs one libgrd public API test case.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, 1 or 2 on failure.
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "test case: expected one argument, got %d\n", argc - 1);
        return 2;
    }
    if (strcmp(argv[1], "version") == 0) return case_version();
    if (strcmp(argv[1], "options-default") == 0) return case_options_default();
    if (strcmp(argv[1], "options-load-env") == 0) return case_options_load_env();
    if (strcmp(argv[1], "options-free") == 0) return case_options_free();
    if (strcmp(argv[1], "box-new") == 0) return case_box_new();
    if (strcmp(argv[1], "box-free") == 0) return case_box_free();
    if (strcmp(argv[1], "split-set") == 0) return case_split_set();
    if (strcmp(argv[1], "split-add") == 0) return case_split_add();
    if (strcmp(argv[1], "split-weight") == 0) return case_split_weight();
    if (strcmp(argv[1], "box-bounds") == 0) return case_box_bounds();
    if (strcmp(argv[1], "box-layout") == 0) return case_box_layout();
    if (strcmp(argv[1], "split-gap") == 0) return case_split_gap();
    if (strcmp(argv[1], "gap-hit") == 0) return case_gap_hit();
    if (strcmp(argv[1], "drag") == 0) return case_drag();
    if (strcmp(argv[1], "box-close") == 0) return case_box_close();
    if (strcmp(argv[1], "split-at") == 0) return case_split_at();
    if (strcmp(argv[1], "stop") == 0) return case_stop();
    if (strcmp(argv[1], "on-signal") == 0) return case_on_signal();
    if (strcmp(argv[1], "raise-signal") == 0) return case_raise_signal();
    if (strcmp(argv[1], "listen-signals") == 0) return case_listen_signals();
    if (strcmp(argv[1], "listen-signal") == 0) return case_listen_signal();
    if (strcmp(argv[1], "signal-listener") == 0) return case_signal_listener();
    if (strcmp(argv[1], "multictx") == 0) return case_multictx();
    fprintf(stderr, "unknown test case: %s\n", argv[1]);
    return 2;
}
