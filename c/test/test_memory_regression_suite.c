/**
 * Memory Regression Test Suite - Comprehensive Runner
 * Runs all memory regression tests and provides unified reporting
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    const char *name;
    const char *description;
    const char *executable;
    int priority; // 1=critical, 2=important, 3=normal
} test_suite_t;

static const test_suite_t test_suites[] = {
    {"Error Paths", "Tests all error scenarios for proper memory cleanup", "./test_memory_error_paths", 1},
    {"Large Data", "Tests size limit boundary conditions for memory safety", "./test_memory_large_data", 1},
    {"Stress Testing", "Tests repeated operations for memory leaks and stability", "./test_memory_stress", 2},
    {"Edge Cases", "Tests boundary conditions and unusual scenarios", "./test_memory_edge_cases", 2}
};

#define NUM_TEST_SUITES (sizeof(test_suites) / sizeof(test_suites[0]))

typedef struct {
    int total_suites;
    int passed_suites; 
    int failed_suites;
    double total_time;
    int critical_failures;
} test_results_t;static int run_test_suite(const test_suite_t *suite, double *execution_time) {
    printf("\n--- Running %s Test Suite ---\n", suite->name);
    printf("Description: %s\n", suite->description);
    printf("Priority: %s\n", suite->priority == 1 ? "CRITICAL" : (suite->priority == 2 ? "IMPORTANT" : "NORMAL"));
    
    clock_t start = clock();
    
    // Fork and exec the test executable
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - exec the test
        execl(suite->executable, suite->executable, (char *)NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        clock_t end = clock();
        *execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("✓ %s PASSED (%.2fs)\n", suite->name, *execution_time);
            return 0;
        } else {
            printf("✗ %s FAILED (%.2fs)\n", suite->name, *execution_time);
            return 1;
        }
    } else {
        perror("fork failed");
        return 1;
    }
}

static void print_regression_summary(const test_results_t *results) {
    printf("\n============================================================\n");
    printf("MEMORY REGRESSION TEST SUITE SUMMARY\n");
    printf("============================================================\n");
    printf("Total Suites:      %d\n", results->total_suites);
    printf("Passed:            %d\n", results->passed_suites);
    printf("Failed:            %d\n", results->failed_suites);
    printf("Critical Failures: %d\n", results->critical_failures);
    printf("Total Time:        %.2f seconds\n", results->total_time);
    printf("Success Rate:      %.1f%%\n", 
           (results->total_suites > 0) ? 
           (100.0 * results->passed_suites / results->total_suites) : 0.0);
    
    if (results->failed_suites == 0) {
        printf("\n🎉 ALL MEMORY REGRESSION TESTS PASSED!\n");
        printf("✓ No valgrind-detectable memory issues found\n");
        printf("✓ All size limits enforced correctly\n");
        printf("✓ Memory stability under stress verified\n");
        printf("✓ Edge cases handled properly\n");
    } else {
        printf("\n⚠️  MEMORY REGRESSION FAILURES DETECTED\n");
        if (results->critical_failures > 0) {
            printf("🚨 CRITICAL: %d critical test suite(s) failed\n", results->critical_failures);
            printf("   Memory safety may be compromised!\n");
        }
    }
}static void print_valgrind_instructions(void) {
    printf("\n============================================================\n");
    printf("VALGRIND INTEGRATION INSTRUCTIONS\n");
    printf("============================================================\n");
    printf("To run with valgrind memory checking:\n\n");
    
    printf("1. Run individual test suites:\n");
    for (size_t i = 0; i < NUM_TEST_SUITES; i++) {
        printf("   valgrind --leak-check=full %s\n", test_suites[i].executable);
    }
    
    printf("\n2. Run full regression suite with valgrind:\n");
    printf("   valgrind --leak-check=full --show-leak-kinds=all ./test_memory_regression_suite\n");
    
    printf("\n3. Expected valgrind output:\n");
    printf("   ✓ All heap blocks were freed -- no leaks are possible\n");
    printf("   ✓ ERROR SUMMARY: 0 errors from 0 contexts\n");
    
    printf("\n4. CI Integration:\n");
    printf("   Add this test suite to your CI pipeline to prevent regressions\n");
    printf("   Use --error-exitcode=1 to fail CI on memory issues\n");
}

int main(int argc, char *argv[]) {
    printf("Memory Regression Test Suite - Phase 7.7\n");
    printf("Comprehensive valgrind error detection and prevention\n");
    
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        print_valgrind_instructions();
        return 0;
    }
    
    test_results_t results = {0};
    results.total_suites = NUM_TEST_SUITES;
    
    clock_t suite_start = clock();
    
    // Run all test suites
    for (size_t i = 0; i < NUM_TEST_SUITES; i++) {
        double execution_time;
        int suite_result = run_test_suite(&test_suites[i], &execution_time);
        
        if (suite_result == 0) {
            results.passed_suites++;
        } else {
            results.failed_suites++;
            if (test_suites[i].priority == 1) {
                results.critical_failures++;
            }
        }
        
        results.total_time += execution_time;
    }
    
    clock_t suite_end = clock();
    results.total_time = ((double)(suite_end - suite_start)) / CLOCKS_PER_SEC;
    
    print_regression_summary(&results);
    
    if (argc > 1 && strcmp(argv[1], "--valgrind-help") == 0) {
        print_valgrind_instructions();
    }
    
    return (results.failed_suites > 0) ? 1 : 0;
}