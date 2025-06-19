#include <stdio.h>
#include "test_framework.h"

int g_tests_run = 0;
int g_tests_failed = 0;

extern int test_build_empty_generate_request(void);
extern int test_build_request_with_empty_options(void);
extern int test_build_request_with_one_string(void);

int main() {
    printf("Running incremental builder tests...\n");
    
    g_tests_run = 0;
    g_tests_failed = 0;
    
    RUN_TEST(build_empty_generate_request);
    RUN_TEST(build_request_with_empty_options);
    RUN_TEST(build_request_with_one_string);
    
    printf("\nTests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
    
    return g_tests_failed > 0 ? 1 : 0;
}