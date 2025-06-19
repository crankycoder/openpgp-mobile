#include <stdio.h>
#include <assert.h>
#include "bridge_builder.h"

void test_options_create_minimal() {
    printf("Test 1: Options_create with all zeros\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    model_Options_ref_t opts = model_Options_create(B, 0, 0, 0, 0, 0);
    printf("Result: %ld\n", (long)opts);
    
    if (opts != 0) {
        printf("Success - created empty Options\n");
    } else {
        printf("FAILED - returned 0\n");
    }
    
    flatcc_builder_clear(B);
}

void test_options_create_strings_only() {
    printf("\nTest 2: Options_create with strings only\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    flatbuffers_string_ref_t name = flatbuffers_string_create_str(B, "Test");
    printf("name ref: %ld\n", (long)name);
    
    model_Options_ref_t opts = model_Options_create(B, name, 0, 0, 0, 0);
    printf("Result: %ld\n", (long)opts);
    
    if (opts != 0) {
        printf("Success - created Options with name\n");
    } else {
        printf("FAILED - returned 0\n");
    }
    
    flatcc_builder_clear(B);
}

void test_options_manual_build() {
    printf("\nTest 3: Options built manually\n");
    
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    assert(flatcc_builder_init(B) == 0);
    
    assert(model_Options_start(B) == 0);
    model_Options_ref_t opts = model_Options_end(B);
    printf("Manual build result: %ld\n", (long)opts);
    
    if (opts != 0) {
        printf("Success - manual build works\n");
    } else {
        printf("FAILED - manual build returned 0\n");
    }
    
    flatcc_builder_clear(B);
}

int main() {
    test_options_create_minimal();
    test_options_create_strings_only();
    test_options_manual_build();
    return 0;
}