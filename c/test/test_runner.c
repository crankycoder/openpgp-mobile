#include "test_framework.h"

/* Global test counters */
int g_tests_run = 0;
int g_tests_failed = 0;

/* Test function declarations */
extern int test_basic_initialization(void);
extern int test_error_handling(void);
extern int test_memory_management(void);
extern int test_helper_functions(void);
extern int test_bridge_integration(void);
extern int test_generate_key_basic(void);
extern int test_generate_key_with_options(void);
extern int test_generate_key_input_validation(void);
extern int test_generate_key_without_init(void);

int main(void) {
    printf(COLOR_BLUE "OpenPGP C Wrapper Library Tests" COLOR_RESET "\n");
    printf("======================================\n\n");

    /* Reset counters */
    g_tests_run = 0;
    g_tests_failed = 0;

    /* Run infrastructure tests */
    RUN_TEST(basic_initialization);
    RUN_TEST(error_handling);
    RUN_TEST(memory_management);
    RUN_TEST(helper_functions);
    RUN_TEST(bridge_integration);
    
    printf("\n" COLOR_BLUE "Key Generation Tests" COLOR_RESET "\n");
    printf("--------------------\n");
    
    /* Run key generation tests */
    RUN_TEST(generate_key_basic);
    RUN_TEST(generate_key_with_options);
    RUN_TEST(generate_key_input_validation);
    RUN_TEST(generate_key_without_init);

    /* Print summary */
    printf("\n======================================\n");
    printf("Tests run: %d\n", g_tests_run);
    printf("Tests failed: %d\n", g_tests_failed);
    
    if (g_tests_failed == 0) {
        printf(COLOR_GREEN "All tests passed!" COLOR_RESET "\n");
        return 0;
    } else {
        printf(COLOR_RED "%d tests failed!" COLOR_RESET "\n", g_tests_failed);
        return 1;
    }
}