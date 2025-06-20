#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "memory_helpers.h"
#include "openpgp.h"
#include <stdio.h>
#include <stdlib.h>

/* Test functions that modify global state */
int test_state_modifier_1(void) {
    openpgp_result_t result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, result.error);
    
    void* ptr = TRACKED_MALLOC(100);
    TEST_ASSERT_NOT_NULL(ptr);
    TRACKED_FREE(ptr);
    
    return 0;
}

int test_state_modifier_2(void) {
    openpgp_result_t result = openpgp_init();
    TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, result.error);
    
    openpgp_cleanup();
    return 0;
}

int test_memory_isolation(void) {
    TEST_ASSERT_EQUAL(0, memory_tracking_get_allocation_count());
    
    void* ptr1 = TRACKED_MALLOC(50);
    void* ptr2 = TRACKED_MALLOC(100);
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    
    TRACKED_FREE(ptr1);
    TRACKED_FREE(ptr2);
    
    return 0;
}/* Main function to test isolation */
int main(void) {
    printf("Testing Test Isolation System\n");
    printf("=============================\n");
    
    /* Initialize isolation system */
    test_isolation_init();
    
    int failures = 0;
    
    /* Run tests in isolated fashion */
    printf("\n--- Testing state isolation ---\n");
    if (RUN_ISOLATED_TEST(state_modifier_1) != 0) failures++;
    if (RUN_ISOLATED_TEST(state_modifier_2) != 0) failures++;
    if (RUN_ISOLATED_TEST(memory_isolation) != 0) failures++;
    
    /* Cleanup isolation system */
    test_isolation_cleanup();
    
    printf("\n=============================\n");
    printf("Tests run: %d, Tests failed: %d, Major tests run: %d, Major tests failed: %d\n",
           get_tests_run(), get_tests_failed(), get_major_tests_run(), get_major_tests_failed());
    
    if (failures == 0) {
        printf("All isolation tests PASSED!\n");
        return 0;
    } else {
        printf("%d isolation tests FAILED!\n", failures);
        return 1;
    }
}