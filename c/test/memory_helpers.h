#ifndef MEMORY_HELPERS_H
#define MEMORY_HELPERS_H

#include <stddef.h>
#include <stdbool.h>

// Memory tracking structure
typedef struct {
    void* ptr;
    size_t size;
    const char* file;
    int line;
    bool freed;
} memory_allocation_t;

// Memory tracking functions
void memory_tracking_init(void);
void memory_tracking_cleanup(void);
void* tracked_malloc(size_t size, const char* file, int line);
void* tracked_calloc(size_t nmemb, size_t size, const char* file, int line);
void* tracked_realloc(void* ptr, size_t size, const char* file, int line);
void tracked_free(void* ptr, const char* file, int line);
char* tracked_strdup(const char* s, const char* file, int line);

// Memory tracking report
void memory_tracking_report(void);
bool memory_tracking_has_leaks(void);
size_t memory_tracking_get_allocated_bytes(void);
size_t memory_tracking_get_allocation_count(void);

// Macros for automatic file/line tracking
#define TRACKED_MALLOC(size) tracked_malloc(size, __FILE__, __LINE__)
#define TRACKED_CALLOC(nmemb, size) tracked_calloc(nmemb, size, __FILE__, __LINE__)
#define TRACKED_REALLOC(ptr, size) tracked_realloc(ptr, size, __FILE__, __LINE__)
#define TRACKED_FREE(ptr) tracked_free(ptr, __FILE__, __LINE__)
#define TRACKED_STRDUP(s) tracked_strdup(s, __FILE__, __LINE__)

// Test isolation helpers
typedef void (*test_function_t)(void);
void run_isolated_test(const char* test_name, test_function_t test_func);

// Valgrind integration helpers
bool is_running_under_valgrind(void);
void valgrind_assert_no_leaks(void);

#endif // MEMORY_HELPERS_H