#include "test_framework.h"
#include "bridge_builder.h"
#include "bridge_reader.h"
#include <string.h>

/* Test 1: Create empty GenerateRequest */
TEST_CASE(build_empty_generate_request) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Build manually */
    TEST_ASSERT_EQUAL(0, model_GenerateRequest_start(B));
    model_GenerateRequest_ref_t req = model_GenerateRequest_end(B);
    TEST_ASSERT(req != 0);
    
    TEST_ASSERT(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    TEST_ASSERT_EQUAL(12, size); /* Empty request is 12 bytes */
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    /* Verify we can parse it */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    /* Options should be null in empty request */
    model_Options_table_t opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NULL(opts);
    
    flatcc_builder_clear(B);
    return 0;
}

/* Test 2: Create GenerateRequest with empty Options */
TEST_CASE(build_request_with_empty_options) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Build empty Options first */
    TEST_ASSERT_EQUAL(0, model_Options_start(B));
    model_Options_ref_t opts = model_Options_end(B);
    TEST_ASSERT(opts != 0);
    
    /* Build GenerateRequest with Options */
    TEST_ASSERT_EQUAL(0, model_GenerateRequest_start(B));
    TEST_ASSERT_EQUAL(0, model_GenerateRequest_options_add(B, opts));
    model_GenerateRequest_ref_t req = model_GenerateRequest_end(B);
    TEST_ASSERT(req != 0);
    
    TEST_ASSERT(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    TEST_ASSERT(size > 12); /* Should be larger than empty */
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    TEST_ASSERT_NOT_NULL(buffer);
    
    /* Parse and verify */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NOT_NULL(parsed_opts);
    
    /* All fields should be null/default */
    TEST_ASSERT_NULL(model_Options_name(parsed_opts));
    TEST_ASSERT_NULL(model_Options_email(parsed_opts));
    TEST_ASSERT_NULL(model_Options_comment(parsed_opts));
    TEST_ASSERT_NULL(model_Options_passphrase(parsed_opts));
    TEST_ASSERT_NULL(model_Options_key_options(parsed_opts));
    
    flatcc_builder_clear(B);
    return 0;
}

/* Test 3: Add one string field */
TEST_CASE(build_request_with_one_string) {
    flatcc_builder_t builder;
    flatcc_builder_t *B = &builder;
    
    TEST_ASSERT_EQUAL(0, flatcc_builder_init(B));
    
    /* Create a string */
    flatbuffers_string_ref_t name_ref = flatbuffers_string_create_str(B, "Test User");
    TEST_ASSERT(name_ref != 0);
    
    /* Build Options with name */
    TEST_ASSERT_EQUAL(0, model_Options_start(B));
    TEST_ASSERT_EQUAL(0, model_Options_name_add(B, name_ref));
    model_Options_ref_t opts = model_Options_end(B);
    
    /* Build GenerateRequest */
    TEST_ASSERT_EQUAL(0, model_GenerateRequest_start(B));
    TEST_ASSERT_EQUAL(0, model_GenerateRequest_options_add(B, opts));
    model_GenerateRequest_ref_t req = model_GenerateRequest_end(B);
    
    TEST_ASSERT(flatcc_builder_end_buffer(B, req) != 0);
    
    size_t size = flatcc_builder_get_buffer_size(B);
    TEST_ASSERT(size > 28); /* Should be larger than empty Options */
    
    void *buffer = flatcc_builder_get_direct_buffer(B, &size);
    
    /* Parse and verify */
    model_GenerateRequest_table_t parsed = model_GenerateRequest_as_root(buffer);
    TEST_ASSERT_NOT_NULL(parsed);
    
    model_Options_table_t parsed_opts = model_GenerateRequest_options(parsed);
    TEST_ASSERT_NOT_NULL(parsed_opts);
    
    const char *parsed_name = model_Options_name(parsed_opts);
    TEST_ASSERT_NOT_NULL(parsed_name);
    TEST_ASSERT_STRING_EQUAL("Test User", parsed_name);
    
    /* Other fields still null */
    TEST_ASSERT_NULL(model_Options_email(parsed_opts));
    
    flatcc_builder_clear(B);
    return 0;
}