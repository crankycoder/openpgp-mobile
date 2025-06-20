#ifndef TEST_ISOLATION_MANAGER_H
#define TEST_ISOLATION_MANAGER_H

#include <stdbool.h>

/* Test isolation manager for coordinating global state reset */

typedef struct {
    int tests_run;
    int tests_failed;
    int major_tests_run;
    int major_tests_failed;
} test_state_t;

/* Test function signature */
typedef int (*isolated_test_function_t)(void);

/* Global state management */
void test_isolation_init(void);
void test_isolation_cleanup(void);

/* Individual test isolation */
int run_isolated_test(isolated_test_function_t test_func, const char* test_name);

/* State snapshot and restore */
test_state_t* test_state_snapshot(void);
void test_state_restore(test_state_t* snapshot);

/* OpenPGP library state reset */
void reset_openpgp_state(void);

/* Memory tracking state reset */
void reset_memory_tracking_state(void);

/* Test counter access (thread-safe) */
int get_tests_run(void);
int get_tests_failed(void);
int get_major_tests_run(void);
int get_major_tests_failed(void);

void increment_tests_run(void);
void increment_tests_failed(void);
void increment_major_tests_run(void);
void increment_major_tests_failed(void);

void reset_test_counters(void);

#endif /* TEST_ISOLATION_MANAGER_H */