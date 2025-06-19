#include <stdio.h>
#include "test_framework.h"

int g_tests_run = 0;
int g_tests_failed = 0;

extern int test_flatbuffer_serialization_simple(void);
extern int test_flatbuffer_serialization_with_strings(void);
extern int test_flatbuffer_serialization_full_request(void);

int main() {
    printf("Running FlatBuffer serialization tests only...\n");
    
    g_tests_run = 0;
    g_tests_failed = 0;
    
    RUN_TEST(flatbuffer_serialization_simple);
    RUN_TEST(flatbuffer_serialization_with_strings);
    RUN_TEST(flatbuffer_serialization_full_request);
    
    printf("\nTests run: %d, failed: %d\n", g_tests_run, g_tests_failed);
    
    return g_tests_failed > 0 ? 1 : 0;
}