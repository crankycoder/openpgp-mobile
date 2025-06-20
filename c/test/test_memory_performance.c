/**
 * Memory Regression Test Suite - Performance Benchmarks
 * Benchmarks memory operations to detect performance regressions
 * Validates that memory fixes don't introduce significant performance overhead
 */

#include "test_framework_isolated.h"
#include "test_isolation_manager.h"
#include "../include/openpgp.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/* Helper macro to free error message and check result */
#define CHECK_RESULT_AND_FREE(result, expected_error) do { \
    if ((result).error_message) { \
        free((result).error_message); \
        (result).error_message = NULL; \
    } \
    TEST_ASSERT_EQUAL((expected_error), (result).error); \
} while(0)

#define BENCHMARK_ITERATIONS 1000
#define PERFORMANCE_THRESHOLD_MS 100.0  // Max 100ms for 1000 operations

// High-resolution timing
static double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

static int benchmark_size_validation_performance(void) {
    printf("Benchmarking size validation performance...\n");
    
    const char *test_message = "Benchmark message for size validation testing";
    const char *password = "benchmark_password";
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric(test_message, password, NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL); // Expected bridge error
    }
    
    double end_time = get_time_ms();
    double elapsed = end_time - start_time;
    double avg_per_op = elapsed / BENCHMARK_ITERATIONS;
    
    printf("Size validation benchmark: %.2fms total, %.3fms/op\n", elapsed, avg_per_op);
    
    // Performance regression check
    TEST_ASSERT(elapsed < PERFORMANCE_THRESHOLD_MS);
    
    printf("Size validation performance benchmark passed\n");
    return 0;
}static int benchmark_error_handling_overhead(void) {
    printf("Benchmarking error handling overhead...\n");
    
    // Benchmark NULL parameter handling
    double start_time = get_time_ms();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        openpgp_result_t result = openpgp_encrypt_symmetric(NULL, "password", NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_INVALID_INPUT);
    }
    
    double end_time = get_time_ms();
    double elapsed = end_time - start_time;
    double avg_per_op = elapsed / BENCHMARK_ITERATIONS;
    
    printf("Error handling benchmark: %.2fms total, %.3fms/op\n", elapsed, avg_per_op);
    
    // Error handling should be very fast
    TEST_ASSERT(elapsed < (PERFORMANCE_THRESHOLD_MS / 2)); // Half the normal threshold
    
    printf("Error handling performance benchmark passed\n");
    return 0;
}

static int benchmark_memory_allocation_patterns(void) {
    printf("Benchmarking memory allocation patterns...\n");
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        // Allocate test data
        char *message = malloc(1024);
        if (message) {
            memset(message, 'B', 1023);
            message[1023] = '\0';
            
            openpgp_result_t result = openpgp_encrypt_symmetric(message, "pass", NULL, NULL);
            CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL);
            
            free(message);
        }
    }
    
    double end_time = get_time_ms();
    double elapsed = end_time - start_time;
    double avg_per_op = elapsed / BENCHMARK_ITERATIONS;
    
    printf("Memory allocation benchmark: %.2fms total, %.3fms/op\n", elapsed, avg_per_op);
    
    // Should not be significantly slower than basic operations
    TEST_ASSERT(elapsed < (PERFORMANCE_THRESHOLD_MS * 2)); // 2x threshold for allocation overhead
    
    printf("Memory allocation performance benchmark passed\n");
    return 0;
}static int benchmark_size_validation_edge_cases(void) {
    printf("Benchmarking size validation edge cases...\n");
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        // Test oversized data (should fail quickly)
        char *large_data = malloc(3000);
        memset(large_data, 'L', 2999);
        large_data[2999] = '\0';
        
        openpgp_result_t result = openpgp_encrypt_symmetric(large_data, "pass", NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_SIZE_LIMIT);
        
        free(large_data);
    }
    
    double end_time = get_time_ms();
    double elapsed = end_time - start_time;
    double avg_per_op = elapsed / BENCHMARK_ITERATIONS;
    
    printf("Size validation edge cases benchmark: %.2fms total, %.3fms/op\n", elapsed, avg_per_op);
    
    // Size limit checks should be very fast
    TEST_ASSERT(elapsed < (PERFORMANCE_THRESHOLD_MS / 2));
    
    printf("Size validation edge cases performance benchmark passed\n");
    return 0;
}

static int benchmark_isolation_overhead(void) {
    printf("Benchmarking test isolation overhead...\n");
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        // Test the overhead of isolation system
        reset_openpgp_state();
        reset_memory_tracking_state();
        
        openpgp_result_t result = openpgp_encrypt_symmetric("test", "pass", NULL, NULL);
        CHECK_RESULT_AND_FREE(result, OPENPGP_ERROR_BRIDGE_CALL);
    }
    
    double end_time = get_time_ms();
    double elapsed = end_time - start_time;
    double avg_per_op = elapsed / BENCHMARK_ITERATIONS;
    
    printf("Test isolation benchmark: %.2fms total, %.3fms/op\n", elapsed, avg_per_op);
    
    // Isolation overhead should be minimal
    TEST_ASSERT(elapsed < PERFORMANCE_THRESHOLD_MS);
    
    printf("Test isolation performance benchmark passed\n");
    return 0;
}int main() {
    printf("\n=== Memory Regression Test Suite - Performance Benchmarks ===\n");
    printf("Benchmarking memory operations for performance regressions\n\n");
    
    int total_failed = 0;
    
    total_failed += run_isolated_test(benchmark_size_validation_performance, "Size Validation Performance");
    total_failed += run_isolated_test(benchmark_error_handling_overhead, "Error Handling Overhead");
    total_failed += run_isolated_test(benchmark_memory_allocation_patterns, "Memory Allocation Patterns");
    total_failed += run_isolated_test(benchmark_size_validation_edge_cases, "Size Validation Edge Cases");
    total_failed += run_isolated_test(benchmark_isolation_overhead, "Test Isolation Overhead");
    
    printf("\n=== Performance Benchmarks Summary ===\n");
    printf("Tests run: %d, Tests failed: %d\n", get_tests_run(), get_tests_failed());
    
    if (total_failed > 0) {
        printf(COLOR_RED "FAILED: %d benchmark(s) failed\n" COLOR_RESET, total_failed);
        printf("Performance regression detected - memory fixes may have introduced overhead\n");
        return 1;
    }
    
    printf(COLOR_GREEN "SUCCESS: All performance benchmarks passed\n" COLOR_RESET);
    printf("Memory fixes have not introduced significant performance overhead\n");
    return 0;
}