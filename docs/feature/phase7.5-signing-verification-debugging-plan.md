# Phase 7.5: Signing/Verification Integration Debugging - Task Plan

## Overview
This phase systematically debugs and fixes the segmentation fault occurring in signing integration tests. The approach starts with the simplest possible operations and gradually adds complexity to isolate the exact point of failure.

## Current Problem
- **Issue**: Segmentation fault in `test_sign_data_with_generated_key()` during integration tests
- **Location**: `/c/test/test_sign_integration.c`
- **Context**: Basic signing operations work, but integration with generated keys causes crash

## Debugging Strategy
1. Use known-good GPG-generated fixture keys from Phase 4
2. Start with minimal test cases
3. Add complexity incrementally
4. Use debugging tools (gdb, valgrind) to identify root cause
5. Fix issue and add regression tests

## Key Resources
- **Fixture Keys**: `/c/test/gpg-test-keys/` (validated in Phase 4 for encryption/decryption)
- **Test Framework**: Unity test framework
- **Debugging Tools**: gdb, valgrind, AddressSanitizer

---

## Tasks

### Task #1: Create minimal test harness for debugging

- Status: not started
- Description: Create dedicated test files for isolated debugging of the segfault. These files will allow running specific tests independently with verbose debug output.
- Acceptance Criteria:
  - Create `/c/test/test_sign_verify_debug.c` with Unity framework setup
  - Create `/c/test/test_sign_verify_minimal.c` for absolute minimal tests
  - Add Makefile target to compile and run debug tests separately
  - Include verbose logging macros for debugging
  - Tests can be enabled/disabled via #define flags
  - Each test is completely isolated (no shared state)
  - Debug output shows memory addresses and sizes
- Assumptions:
  - Unity test framework is available and working
  - Can create separate test executables
- Dependencies:
  - None
- Reference files:
  - `/c/test/test_sign_integration.c` (where segfault occurs)
  - `/c/test/test_helper.h`
  - `/c/Makefile`
- Examples for implementing:
  - Good example:
    ```c
    // Debug test with verbose output
    void test_minimal_sign_debug(void) {
        printf("DEBUG: Starting minimal sign test\n");
        printf("DEBUG: Allocating key buffer...\n");
        const char* key = TEST_PRIVATE_KEY_NO_PASSPHRASE;
        printf("DEBUG: Key address: %p, length: %zu\n", key, strlen(key));
        
        printf("DEBUG: Calling openpgp_sign...\n");
        openpgp_result_t result = openpgp_sign("test", key, NULL);
        printf("DEBUG: Sign returned, error=%d\n", result.error);
        
        if (result.data) {
            printf("DEBUG: Result data: %p, length: %zu\n", 
                   result.data, strlen(result.data));
            free(result.data);
        }
    }
    ```
  - Bad example:
    ```c
    // Don't mix multiple operations in debug tests
    void test_everything(void) {
        generate_key();
        sign_message();
        verify_signature();
        // Too complex - can't isolate failure point
    }
    ```

### Task #2: Test bridge communication without cryptographic operations

- Status: not started
- Description: Verify that basic FlatBuffer bridge calls work correctly without any signing/verification operations. This isolates bridge communication from crypto operations.
- Acceptance Criteria:
  - Create test that builds minimal FlatBuffer request
  - Test empty/minimal bridge calls
  - Verify FlatBuffer builder creation/destruction
  - Check memory allocation patterns
  - Test buffer size limits
  - Ensure no segfaults on basic operations
  - Add tests for malformed requests
- Assumptions:
  - Bridge supports some form of echo or status operation
  - FlatBuffer infrastructure is functional
- Dependencies:
  - Task #1 (need test harness)
- Reference files:
  - `/c/src/openpgp.c` (bridge call implementation)
  - `/c/generated/bridge_builder.h`
  - `/flatbuffers/bridge.fbs`
- Examples for implementing:
  - Good example:
    ```c
    void test_bridge_minimal_flatbuffer(void) {
        printf("DEBUG: Creating FlatBuffer builder\n");
        flatbuffers_builder_t *B = flatbuffers_builder_create(256);
        TEST_ASSERT_NOT_NULL(B);
        
        printf("DEBUG: Building empty request\n");
        model_SignRequest_start(B);
        model_SignRequest_ref_t request = model_SignRequest_end(B);
        
        size_t size;
        uint8_t *buffer = flatbuffers_builder_finalize(B, request, &size);
        printf("DEBUG: Buffer size: %zu\n", size);
        
        flatbuffers_builder_destroy(B);
        printf("DEBUG: Builder destroyed successfully\n");
    }
    ```

### Task #3: Test minimal signing operation with hardcoded keys

- Status: not started
- Description: Test the simplest possible signing operations using hardcoded fixture keys, starting with empty messages and progressing to simple strings.
- Acceptance Criteria:
  - Use GPG fixture key from `/c/test/gpg-test-keys/test_key_rsa_no_pass.h`
  - Test signing empty string ""
  - Test signing single character "a"
  - Test signing "test" message
  - Verify each step completes without segfault
  - Check return values and error messages
  - Validate memory is properly freed
  - Add debug output at each step
- Assumptions:
  - Fixture keys are valid and properly formatted
  - Signing empty strings is supported
- Dependencies:
  - Task #2 (verify bridge works)
- Reference files:
  - `/c/test/gpg-test-keys/test_key_rsa_no_pass.h`
  - `/c/src/openpgp_sign.c`
  - `/c/test/test_sign.c`
- Examples for implementing:
  - Good example:
    ```c
    void test_sign_empty_string_debug(void) {
        printf("DEBUG: Test signing empty string\n");
        
        // Use known-good fixture key
        const char* private_key = TEST_PRIVATE_KEY_RSA_NO_PASS;
        TEST_ASSERT_NOT_NULL(private_key);
        
        printf("DEBUG: Signing empty string...\n");
        openpgp_result_t result = openpgp_sign("", private_key, NULL);
        
        printf("DEBUG: Result - error: %d, data: %p\n", 
               result.error, result.data);
        
        if (result.error == OPENPGP_SUCCESS) {
            TEST_ASSERT_NOT_NULL(result.data);
            TEST_ASSERT_TRUE(strstr(result.data, "BEGIN PGP SIGNATURE"));
            free(result.data);
        }
    }
    ```

### Task #4: Test key memory management in signing operations

- Status: not started
- Description: Isolate and test how keys are handled in memory during signing operations. This helps identify if the segfault is related to key memory management.
- Acceptance Criteria:
  - Test passing NULL key (should fail gracefully)
  - Test empty key string "" (should fail gracefully)
  - Test malformed key data
  - Test very large key (memory stress)
  - Test key string that's not null-terminated
  - Monitor memory allocation/deallocation
  - Use valgrind to check for memory errors
  - Ensure no buffer overflows
- Assumptions:
  - Invalid keys should be rejected before signing
  - Memory validation tools are available
- Dependencies:
  - Task #3 (basic signing works)
- Reference files:
  - `/c/src/openpgp_sign.c`
  - `/c/src/openpgp.c` (memory management helpers)
- Examples for implementing:
  - Good example:
    ```c
    void test_sign_null_key_handling(void) {
        printf("DEBUG: Testing NULL key handling\n");
        
        openpgp_result_t result = openpgp_sign("test", NULL, NULL);
        TEST_ASSERT_EQUAL(OPENPGP_ERROR_INVALID_INPUT, result.error);
        TEST_ASSERT_NULL(result.data);
        
        printf("DEBUG: NULL key handled correctly\n");
    }
    
    void test_sign_malformed_key(void) {
        printf("DEBUG: Testing malformed key\n");
        
        const char* bad_key = "NOT A VALID PGP KEY";
        openpgp_result_t result = openpgp_sign("test", bad_key, NULL);
        TEST_ASSERT_NOT_EQUAL(OPENPGP_SUCCESS, result.error);
        
        if (result.error_message) {
            printf("DEBUG: Error message: %s\n", result.error_message);
            free(result.error_message);
        }
    }
    ```

### Task #5: Debug generated key integration point

- Status: not started
- Description: Carefully test the integration between key generation and signing, as this is where the segfault occurs. Use debugging tools to identify the exact failure point.
- Acceptance Criteria:
  - Generate minimal RSA key (2048-bit)
  - Extract private key string immediately after generation
  - Validate key string format and length
  - Pass to sign function with debug output
  - Use gdb to get stack trace at segfault
  - Check for memory corruption with valgrind
  - Test with different message sizes
  - Document exact line where segfault occurs
- Assumptions:
  - Key generation itself works correctly
  - Can attach debugger to test process
- Dependencies:
  - Task #4 (key memory handling understood)
- Reference files:
  - `/c/test/test_sign_integration.c` (failing test)
  - `/c/src/openpgp_generate.c`
  - `/c/src/openpgp_sign.c`
- Examples for implementing:
  - Good example:
    ```c
    void test_generated_key_sign_debug(void) {
        printf("DEBUG: Generating RSA key...\n");
        
        openpgp_options_t opts = {
            .email = "test@example.com",
            .name = "Debug Test",
            .passphrase = NULL  // No passphrase for simplicity
        };
        
        openpgp_key_options_t key_opts = {
            .algorithm = OPENPGP_ALGORITHM_RSA,
            .rsa_bits = 2048
        };
        
        openpgp_result_t gen_result = openpgp_generate_key_with_options(&opts, &key_opts);
        TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, gen_result.error);
        TEST_ASSERT_NOT_NULL(gen_result.keypair);
        
        printf("DEBUG: Key generated, private key length: %zu\n", 
               strlen(gen_result.keypair->private_key));
        
        // Create a copy to isolate memory issues
        char* key_copy = strdup(gen_result.keypair->private_key);
        TEST_ASSERT_NOT_NULL(key_copy);
        
        printf("DEBUG: Attempting to sign with generated key...\n");
        openpgp_result_t sign_result = openpgp_sign("test", key_copy, NULL);
        printf("DEBUG: Sign returned: error=%d\n", sign_result.error);
        
        // Cleanup
        free(key_copy);
        openpgp_free_keypair(gen_result.keypair);
        if (sign_result.data) free(sign_result.data);
    }
    ```

### Task #6: Run debugging tools to identify root cause

- Status: not started
- Description: Use gdb, valgrind, and other debugging tools to get detailed information about the segfault and identify the exact cause.
- Acceptance Criteria:
  - Get full stack trace with gdb at segfault point
  - Run valgrind with --leak-check=full --track-origins=yes
  - Check for buffer overflows with AddressSanitizer
  - Document exact function and line causing segfault
  - Identify type of memory error (overflow, use-after-free, etc.)
  - Check FlatBuffer size calculations
  - Verify string null-termination
  - Create minimal reproducible test case
- Assumptions:
  - Debug symbols are available (-g flag)
  - Debugging tools are installed
- Dependencies:
  - Task #5 (reproducible test case)
- Reference files:
  - `/c/Makefile` (add debug flags if needed)
  - Core dump files
- Examples for implementing:
  - Good debugging commands:
    ```bash
    # Run with gdb
    gdb ./build/test_debug
    (gdb) run
    (gdb) bt full  # when segfault occurs
    
    # Run with valgrind
    valgrind --leak-check=full --track-origins=yes ./build/test_debug
    
    # Compile with AddressSanitizer
    gcc -fsanitize=address -g -o test_debug test_sign_verify_debug.c
    ```

### Task #7: Implement fix for identified issue

- Status: not started
- Description: Based on debugging findings, implement the minimal fix needed to resolve the segfault. The fix should be targeted and well-documented.
- Acceptance Criteria:
  - Fix addresses root cause identified in Task #6
  - Add defensive programming (null checks, bounds checks)
  - Update error handling if needed
  - Ensure fix doesn't break existing functionality
  - Add code comments explaining the fix
  - Test fix with minimal test case
  - Verify no segfault with original failing test
  - Run full test suite to check for regressions
- Assumptions:
  - Root cause has been clearly identified
  - Fix is localized to specific function(s)
- Dependencies:
  - Task #6 (root cause identified)
- Reference files:
  - Files identified as problematic in Task #6
- Examples for implementing:
  - Good example (hypothetical fix):
    ```c
    // Before: Missing null check
    size_t key_len = strlen(private_key);
    
    // After: Add defensive checks
    if (!private_key) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT, 
                                 "Private key cannot be NULL");
    }
    size_t key_len = strlen(private_key);
    
    // Add bounds check for FlatBuffer
    if (key_len > MAX_KEY_SIZE) {
        return create_error_result(OPENPGP_ERROR_INVALID_INPUT,
                                 "Private key exceeds maximum size");
    }
    ```

### Task #8: Add regression tests

- Status: not started
- Description: Create comprehensive regression tests to ensure the segfault issue doesn't reoccur. These tests should cover the specific failure case and related edge cases.
- Acceptance Criteria:
  - Create specific test for the original failure scenario
  - Add edge case tests around the fix
  - Include memory stress tests
  - Test with various key sizes and types
  - Add tests to main test suite
  - Document what issue each test prevents
  - Ensure tests fail without the fix
  - All tests pass with the fix
- Assumptions:
  - Fix has been implemented and verified
- Dependencies:
  - Task #7 (fix implemented)
- Reference files:
  - `/c/test/test_sign_integration.c`
  - `/c/test/test_sign.c`
- Examples for implementing:
  - Good example:
    ```c
    // Regression test for segfault issue
    void test_sign_with_generated_key_regression(void) {
        // This test prevents regression of issue #XXX
        // Previously caused segfault due to [specific reason]
        
        // Generate key
        openpgp_result_t gen = generate_test_keypair();
        TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, gen.error);
        
        // Sign with generated key - this used to segfault
        openpgp_result_t sign = openpgp_sign("regression test", 
                                           gen.keypair->private_key, 
                                           NULL);
        TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, sign.error);
        TEST_ASSERT_NOT_NULL(sign.data);
        
        // Cleanup
        openpgp_free_keypair(gen.keypair);
        free(sign.data);
    }
    ```

### Task #9: Update documentation and cleanup

- Status: not started
- Description: Document the debugging process, the issue found, and the fix applied. Clean up debug code and update the main plan.
- Acceptance Criteria:
  - Create `/docs/debugging/segfault-sign-integration.md` with:
    - Description of the issue
    - Debugging steps taken
    - Root cause analysis
    - Fix explanation
    - Lessons learned
  - Remove or ifdef debug printf statements
  - Update main plan.md to reflect completion
  - Add comments in code about the fix
  - Update CHANGELOG if needed
  - Ensure all tests pass
  - Commit all changes
- Assumptions:
  - All previous tasks completed successfully
- Dependencies:
  - Tasks #1-8 (all debugging and fixes complete)
- Reference files:
  - `/docs/plan.md`
  - `/c/README.md`
  - Fixed source files
- Examples for implementing:
  - Good documentation:
    ```markdown
    ## Issue Description
    Segmentation fault occurred when signing with keys generated by our API.
    
    ## Root Cause
    The private key string from key generation was not properly null-terminated
    when passed to the FlatBuffer builder, causing buffer overflow.
    
    ## Fix
    Added explicit null-termination check and bounds validation in 
    openpgp_sign() before building FlatBuffer request.
    
    ## Prevention
    - Always validate string inputs before FlatBuffer operations
    - Use safe string functions (strnlen instead of strlen)
    - Add regression test for this specific case
    ```

## Success Criteria

1. Segmentation fault is eliminated
2. All existing tests continue to pass
3. Integration tests work with generated keys
4. Root cause is documented
5. Regression tests prevent reoccurrence
6. Memory leaks are verified to be absent
7. Performance is not significantly impacted

## Testing Strategy

1. Start with minimal isolated tests
2. Use debugging tools at each stage
3. Build complexity incrementally
4. Verify fix with original failing test
5. Run full test suite for regression check
6. Use valgrind for memory verification