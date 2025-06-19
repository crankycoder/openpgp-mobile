#include <stdio.h>
#include "test_framework.h"

int g_tests_run = 0;
int g_tests_failed = 0;

extern int test_generate_key_basic(void);

int main() {
    printf("Running generate key test only...\n");
    
    g_tests_run = 0;
    g_tests_failed = 0;
    
    RUN_TEST(generate_key_basic);
    
    printf("\nTests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
    
    return g_tests_failed > 0 ? 1 : 0;
}