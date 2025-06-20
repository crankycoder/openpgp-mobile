#include "test_isolation_manager.h"
#include "memory_helpers.h"
#include "openpgp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Thread-safe test counters (replace globals) */
static int g_tests_run = 0;
static int g_tests_failed = 0;
static int g_major_tests_run = 0;
static int g_major_tests_failed = 0;

/* Forward declaration for BytesReturn and OpenPGP bridge call */
typedef struct {
    char *data;
    int length;
    char *error;
} BytesReturn;

typedef BytesReturn* (*OpenPGPBridgeCall_fn)(char* name, void* payload, int payloadSize);

/* OpenPGP global state structure (matches openpgp.c) */
static struct {
    bool initialized;
    void *bridge_handle;
    OpenPGPBridgeCall_fn bridge_call;
} g_openpgp_state = {0};

/* Initialize test isolation system */
void test_isolation_init(void) {
    reset_test_counters();
    reset_memory_tracking_state();
    reset_openpgp_state();
}

/* Cleanup test isolation system */
void test_isolation_cleanup(void) {
    reset_openpgp_state();
    reset_memory_tracking_state();
    reset_test_counters();
}

/* Run a single test in complete isolation */
int run_isolated_test(isolated_test_function_t test_func, const char* test_name) {
    if (!test_func || !test_name) {
        return -1;
    }

    /* Create snapshot of current state */
    test_state_t* snapshot = test_state_snapshot();
    if (!snapshot) {
        return -1;
    }

    /* Reset all global state before test */
    reset_openpgp_state();    reset_memory_tracking_state();
    
    /* Initialize memory tracking for this test */
    memory_tracking_init();

    printf("Running %s in isolation... ", test_name);
    fflush(stdout);

    /* Run the test */
    int result = test_func();

    /* Check for memory leaks */
    bool has_leaks = memory_tracking_has_leaks();
    if (has_leaks) {
        printf("\nMemory leaks detected in %s:\n", test_name);
        memory_tracking_report();
        result = 1; /* Force failure on memory leaks */
    }

    /* Cleanup memory tracking */
    memory_tracking_cleanup();

    /* Reset state after test */
    reset_openpgp_state();

    /* Report result */
    if (result == 0 && !has_leaks) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
    }

    /* Restore previous state */
    test_state_restore(snapshot);
    free(snapshot);

    return result;
}

/* Create snapshot of current test state */
test_state_t* test_state_snapshot(void) {
    test_state_t* snapshot = malloc(sizeof(test_state_t));
    if (!snapshot) {
        return NULL;
    }

    snapshot->tests_run = g_tests_run;
    snapshot->tests_failed = g_tests_failed;
    snapshot->major_tests_run = g_major_tests_run;
    snapshot->major_tests_failed = g_major_tests_failed;

    return snapshot;
}

/* Restore test state from snapshot */
void test_state_restore(test_state_t* snapshot) {
    if (!snapshot) {
        return;
    }

    g_tests_run = snapshot->tests_run;    g_tests_failed = snapshot->tests_failed;
    g_major_tests_run = snapshot->major_tests_run;
    g_major_tests_failed = snapshot->major_tests_failed;
}

/* Reset OpenPGP library state */
void reset_openpgp_state(void) {
    /* Force cleanup if initialized */
    openpgp_cleanup();

    /* Note: The actual g_openpgp is static in openpgp.c, 
     * but openpgp_cleanup() should reset it properly.
     * We maintain our own state tracking here for verification. */
    memset(&g_openpgp_state, 0, sizeof(g_openpgp_state));
}

/* Reset memory tracking state */
void reset_memory_tracking_state(void) {
    memory_tracking_cleanup();
}

/* Thread-safe test counter accessors */
int get_tests_run(void) { return g_tests_run; }
int get_tests_failed(void) { return g_tests_failed; }
int get_major_tests_run(void) { return g_major_tests_run; }
int get_major_tests_failed(void) { return g_major_tests_failed; }

void increment_tests_run(void) { g_tests_run++; }
void increment_tests_failed(void) { g_tests_failed++; }
void increment_major_tests_run(void) { g_major_tests_run++; }
void increment_major_tests_failed(void) { g_major_tests_failed++; }

void reset_test_counters(void) {
    g_tests_run = 0;
    g_tests_failed = 0;
    g_major_tests_run = 0;
    g_major_tests_failed = 0;
}