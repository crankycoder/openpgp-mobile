#include <stdio.h>
#include "test_framework.h"

int g_tests_run = 0;
int g_tests_failed = 0;

extern int test_basic_initialization(void);
extern int test_generate_key_basic(void);
extern int test_create_generate_request(void);

int main() {
    printf("Running minimal test set...\n");
    
    g_tests_run = 0;
    g_tests_failed = 0;
    
    printf("\nTest 1: Basic initialization\n");
    RUN_TEST(basic_initialization);
    
    printf("\nTest 2: Generate key\n");
    RUN_TEST(generate_key_basic);
    
    printf("\nTest 3: Create generate request\n");
    RUN_TEST(create_generate_request);
    
    printf("\nTests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
    
    return g_tests_failed > 0 ? 1 : 0;
}