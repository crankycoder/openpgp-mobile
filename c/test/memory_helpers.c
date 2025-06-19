#include "memory_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Maximum tracked allocations
#define MAX_ALLOCATIONS 10000

// Global tracking state
static memory_allocation_t allocations[MAX_ALLOCATIONS];
static size_t allocation_count = 0;
static size_t total_allocated = 0;
static bool tracking_enabled = false;

void memory_tracking_init(void) {
    memset(allocations, 0, sizeof(allocations));
    allocation_count = 0;
    total_allocated = 0;
    tracking_enabled = true;
}

void memory_tracking_cleanup(void) {
    tracking_enabled = false;
    
    // Report any leaks before cleanup
    if (memory_tracking_has_leaks()) {
        memory_tracking_report();
    }
    
    // Reset state
    memset(allocations, 0, sizeof(allocations));
    allocation_count = 0;
    total_allocated = 0;
}

static memory_allocation_t* find_allocation(void* ptr) {
    if (!ptr) return NULL;
    
    for (size_t i = 0; i < allocation_count; i++) {
        if (allocations[i].ptr == ptr && !allocations[i].freed) {
            return &allocations[i];
        }
    }
    return NULL;
}

static memory_allocation_t* add_allocation(void* ptr, size_t size, const char* file, int line) {
    if (!tracking_enabled || !ptr) return NULL;
    
    if (allocation_count >= MAX_ALLOCATIONS) {
        fprintf(stderr, "ERROR: Too many allocations to track\n");
        return NULL;
    }
    
    memory_allocation_t* alloc = &allocations[allocation_count++];
    alloc->ptr = ptr;
    alloc->size = size;
    alloc->file = file;
    alloc->line = line;
    alloc->freed = false;
    
    total_allocated += size;
    
    return alloc;
}void* tracked_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);
    add_allocation(ptr, size, file, line);
    return ptr;
}

void* tracked_calloc(size_t nmemb, size_t size, const char* file, int line) {
    void* ptr = calloc(nmemb, size);
    add_allocation(ptr, nmemb * size, file, line);
    return ptr;
}

void* tracked_realloc(void* old_ptr, size_t size, const char* file, int line) {
    // Remove old allocation
    memory_allocation_t* old_alloc = find_allocation(old_ptr);
    if (old_alloc) {
        total_allocated -= old_alloc->size;
        old_alloc->freed = true;
    }
    
    // Add new allocation
    void* ptr = realloc(old_ptr, size);
    add_allocation(ptr, size, file, line);
    return ptr;
}

void tracked_free(void* ptr, const char* file, int line) {
    if (!ptr) return;
    
    memory_allocation_t* alloc = find_allocation(ptr);
    if (!alloc) {
        if (tracking_enabled) {
            fprintf(stderr, "WARNING: Freeing untracked pointer %p at %s:%d\n", 
                    ptr, file, line);
        }
    } else {
        total_allocated -= alloc->size;
        alloc->freed = true;
    }
    
    free(ptr);
}

char* tracked_strdup(const char* s, const char* file, int line) {
    if (!s) return NULL;
    
    size_t len = strlen(s) + 1;
    char* ptr = malloc(len);
    if (ptr) {
        strcpy(ptr, s);
        add_allocation(ptr, len, file, line);
    }
    return ptr;
}void memory_tracking_report(void) {
    printf("\n=== Memory Tracking Report ===\n");
    printf("Total allocated: %zu bytes\n", total_allocated);
    printf("Active allocations: %zu\n", allocation_count);
    
    size_t leaked_count = 0;
    size_t leaked_bytes = 0;
    
    for (size_t i = 0; i < allocation_count; i++) {
        if (!allocations[i].freed) {
            leaked_count++;
            leaked_bytes += allocations[i].size;
            printf("LEAK: %zu bytes at %p (%s:%d)\n",
                   allocations[i].size, allocations[i].ptr,
                   allocations[i].file, allocations[i].line);
        }
    }
    
    if (leaked_count > 0) {
        printf("\nTOTAL LEAKS: %zu allocations, %zu bytes\n", 
               leaked_count, leaked_bytes);
    } else {
        printf("\nNo memory leaks detected!\n");
    }
    printf("==============================\n\n");
}

bool memory_tracking_has_leaks(void) {
    for (size_t i = 0; i < allocation_count; i++) {
        if (!allocations[i].freed) {
            return true;
        }
    }
    return false;
}

size_t memory_tracking_get_allocated_bytes(void) {
    return total_allocated;
}

size_t memory_tracking_get_allocation_count(void) {
    size_t active = 0;
    for (size_t i = 0; i < allocation_count; i++) {
        if (!allocations[i].freed) {
            active++;
        }
    }
    return active;
}