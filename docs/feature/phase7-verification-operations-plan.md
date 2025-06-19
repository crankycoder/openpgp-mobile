# Phase 7: Verification Operations - Task Plan

## Overview
This phase implements signature verification functionality for the OpenPGP C bindings. The verification functions will validate signatures created by the signing operations from Phase 6, extract original data from signed messages, and provide comprehensive error handling.

## Implementation Strategy
- Follow the established FlatBuffer-based pattern from previous phases
- Ensure compatibility with signatures created by both C and Go implementations
- Implement proper memory management for extracted data
- Use Test-Driven Development (TDD) approach

---

## Tasks

### Task #1: Add verification function declarations to C API header

- Status: not started
- Description: Define the public API for signature verification operations in the C header file. This includes functions for verifying signed messages, detached signatures, file signatures, and raw byte signatures.
- Acceptance Criteria:
  - Add `openpgp_verify()` declaration for verifying signed messages
  - Add `openpgp_verify_data()` declaration for verifying detached signatures
  - Add `openpgp_verify_file()` declaration for verifying file signatures
  - Add `openpgp_verify_bytes()` declaration for verifying byte signatures
  - Define `openpgp_verification_result_t` structure with fields for:
    - Verification status (valid/invalid)
    - Signer information (key ID, fingerprint)
    - Extracted original data (for signed messages)
    - Error details
  - Add documentation comments for each function
  - Include memory management functions for verification results
- Assumptions:
  - Function signatures follow the pattern established in previous phases
  - Return type is `openpgp_result_t` with verification details in result structure
- Dependencies:
  - None
- Reference files:
  - `/c/include/openpgp.h`
  - `/c/include/openpgp_types.h`
- Examples for implementing:
  - Good example:
    ```c
    // Verify a signed message and extract original content
    openpgp_result_t openpgp_verify(
        const char* signed_message,
        const char* public_key,
        openpgp_verification_result_t** result
    );
    ```
  - Bad example:
    ```c
    // Don't return just boolean - we need detailed verification info
    bool openpgp_verify(const char* message, const char* key);
    ```

### Task #2: Create test file for verification operations

- Status: not started
- Description: Create comprehensive test suite for signature verification functionality using the Unity testing framework. Tests should cover all verification scenarios including valid signatures, invalid signatures, wrong keys, and error cases.
- Acceptance Criteria:
  - Create `/c/test/test_verify.c` with Unity test framework
  - Import test helper functions from existing test files
  - Create test fixtures with:
    - Valid signed messages from Phase 6 tests
    - Known good signatures from Go implementation
    - Test public/private key pairs
  - Write placeholder tests for each verification function
  - Ensure tests compile but expect failures initially (TDD approach)
- Assumptions:
  - Unity test framework is already set up (from previous phases)
  - Test signatures from Phase 6 are available for verification
- Dependencies:
  - Task #1 (need API declarations)
- Reference files:
  - `/c/test/test_sign.c` (for signature test data)
  - `/c/test/test_helper.h`
  - `/c/test/unity.h`
- Examples for implementing:
  - Good example:
    ```c
    void test_verify_valid_signed_message(void) {
        // Arrange
        const char* signed_msg = get_test_signed_message();
        const char* public_key = get_test_public_key();
        
        // Act
        openpgp_verification_result_t* result = NULL;
        openpgp_result_t res = openpgp_verify(signed_msg, public_key, &result);
        
        // Assert
        TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, res.error);
        TEST_ASSERT_NOT_NULL(result);
        TEST_ASSERT_TRUE(result->is_valid);
        TEST_ASSERT_NOT_NULL(result->original_data);
    }
    ```

### Task #3: Implement openpgp_verify() for signed messages

- Status: not started
- Description: Implement the core verification function for signed messages. This function should verify the signature, extract the original message content, and provide signer information.
- Acceptance Criteria:
  - Parse the signed message to extract signature and content
  - Create FlatBuffer VerifyRequest with signed message and public key
  - Call OpenPGPBridgeCall("verify", buffer, size)
  - Parse VerifyResponse FlatBuffer to extract:
    - Verification status
    - Original message content
    - Signer key ID/fingerprint
    - Error details if verification fails
  - Allocate and populate openpgp_verification_result_t
  - Handle all error cases (invalid format, wrong key, etc.)
  - Ensure proper memory management
- Assumptions:
  - FlatBuffer schema includes VerifyRequest/VerifyResponse messages
  - Bridge supports "verify" operation
- Dependencies:
  - Task #1 (API declarations)
  - Task #2 (tests to verify implementation)
- Reference files:
  - `/c/src/openpgp.c`
  - `/flatbuffers/bridge.fbs`
  - `/c/generated/bridge_builder.h`
  - `/c/generated/bridge_reader.h`
- Examples for implementing:
  - Good example:
    ```c
    // Build FlatBuffer request
    flatbuffers_builder_t *B = flatbuffers_builder_create(1024);
    
    flatcc_string_ref_t message_ref = flatbuffers_string_create(B, signed_message, strlen(signed_message));
    flatcc_string_ref_t key_ref = flatbuffers_string_create(B, public_key, strlen(public_key));
    
    model_VerifyRequest_start(B);
    model_VerifyRequest_message_add(B, message_ref);
    model_VerifyRequest_public_key_add(B, key_ref);
    model_VerifyRequest_ref_t request = model_VerifyRequest_end(B);
    ```

### Task #4: Implement openpgp_verify_data() for detached signatures

- Status: not started
- Description: Implement verification for detached signatures where the signature and data are provided separately. This is commonly used for file signatures and data integrity verification.
- Acceptance Criteria:
  - Accept separate data and signature parameters
  - Create FlatBuffer VerifyDataRequest with data, signature, and public key
  - Call OpenPGPBridgeCall("verifyData", buffer, size)
  - Parse response to determine verification status
  - Return signer information but no extracted data (since data is provided)
  - Handle binary data correctly (not just text)
  - Implement proper error handling
- Assumptions:
  - Detached signatures are in armored format
  - Bridge supports "verifyData" operation
- Dependencies:
  - Task #3 (can reuse parsing/error handling patterns)
- Reference files:
  - `/c/src/openpgp.c`
  - `/flatbuffers/bridge.fbs`
- Examples for implementing:
  - Good example:
    ```c
    openpgp_result_t openpgp_verify_data(
        const void* data,
        size_t data_len,
        const char* signature,
        const char* public_key,
        openpgp_verification_result_t** result
    ) {
        // Handle both text and binary data
        // Signature is separate from data
    }
    ```

### Task #5: Implement openpgp_verify_file() for file signatures

- Status: not started
- Description: Implement file signature verification that reads a file and verifies its detached signature. This should handle large files efficiently without loading entire content into memory.
- Acceptance Criteria:
  - Accept file path and signature (either inline or detached)
  - Read file content efficiently (consider streaming for large files)
  - Create appropriate FlatBuffer request
  - Call bridge with file data and signature
  - Return verification status with file metadata
  - Handle file I/O errors gracefully
  - Support both armored and binary signature formats
- Assumptions:
  - For initial implementation, files can be loaded entirely into memory
  - Streaming support can be added later if needed
- Dependencies:
  - Task #4 (builds on detached signature verification)
- Reference files:
  - `/c/src/openpgp.c`
  - `/c/test/test_symmetric.c` (for file handling examples)
- Examples for implementing:
  - Good example:
    ```c
    // Read file content
    FILE* f = fopen(file_path, "rb");
    if (!f) return create_error_result(OPENPGP_ERROR_FILE_NOT_FOUND);
    
    // Get file size and read content
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    ```

### Task #6: Implement openpgp_verify_bytes() for raw byte verification

- Status: not started
- Description: Implement verification for raw byte arrays with their signatures. This is useful for verifying binary data, API responses, or any non-text content.
- Acceptance Criteria:
  - Accept byte array, length, signature, and public key
  - Handle binary data correctly (no string assumptions)
  - Create appropriate FlatBuffer request
  - Return verification status
  - Ensure no buffer overflows or memory issues
  - Support various signature formats
- Assumptions:
  - Similar to verify_data but with explicit byte handling
  - No text encoding assumptions
- Dependencies:
  - Task #4 (similar to detached signature verification)
- Reference files:
  - `/c/src/openpgp.c`
  - `/c/test/test_symmetric.c` (for byte handling examples)
- Examples for implementing:
  - Good example:
    ```c
    // Handle raw bytes without string assumptions
    openpgp_result_t openpgp_verify_bytes(
        const uint8_t* data,
        size_t data_len,
        const char* signature,
        const char* public_key,
        openpgp_verification_result_t** result
    )
    ```

### Task #7: Create integration tests for verification

- Status: not started
- Description: Create comprehensive integration tests that verify signatures created by both the C implementation (Phase 6) and the Go implementation. This ensures full interoperability.
- Acceptance Criteria:
  - Create `/c/test/test_verify_integration.c`
  - Test verification of signatures created by:
    - C signing functions (all variants)
    - Go implementation test vectors
    - GPG command-line tool (if available)
  - Test cross-algorithm verification:
    - RSA signatures with RSA keys
    - ECDSA signatures with ECDSA keys
    - EdDSA signatures with Ed25519 keys
  - Test error scenarios:
    - Wrong public key
    - Corrupted signature
    - Modified message content
  - Verify memory management with valgrind
- Assumptions:
  - Test signatures from various sources are available
  - Integration with Go test suite is possible
- Dependencies:
  - Tasks #3-6 (all verification functions implemented)
- Reference files:
  - `/c/test/test_sign_integration.c`
  - `/c/test/test_keygen_integration.c`
- Examples for implementing:
  - Good example:
    ```c
    void test_verify_c_generated_signatures(void) {
        // Generate signature with C API
        openpgp_result_t sign_result = openpgp_sign("test", private_key, passphrase);
        
        // Verify with C API
        openpgp_verification_result_t* verify_result = NULL;
        openpgp_result_t res = openpgp_verify(sign_result.data, public_key, &verify_result);
        
        TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, res.error);
        TEST_ASSERT_TRUE(verify_result->is_valid);
    }
    ```

### Task #8: Add memory management and cleanup functions

- Status: not started
- Description: Implement proper memory management for verification results, including cleanup functions and memory leak prevention.
- Acceptance Criteria:
  - Implement `openpgp_free_verification_result()` function
  - Ensure all allocated strings are properly freed
  - Add NULL checks in cleanup functions
  - Update all verification functions to set pointers to NULL after free
  - Document memory ownership clearly
  - Test with valgrind for memory leaks
- Assumptions:
  - Following same memory management patterns as previous phases
- Dependencies:
  - Tasks #3-6 (verification functions that allocate memory)
- Reference files:
  - `/c/src/openpgp.c` (existing memory management functions)
  - `/c/include/openpgp.h`
- Examples for implementing:
  - Good example:
    ```c
    void openpgp_free_verification_result(openpgp_verification_result_t* result) {
        if (result) {
            free(result->original_data);
            free(result->signer_key_id);
            free(result->signer_fingerprint);
            free(result->error_details);
            free(result);
        }
    }
    ```

### Task #9: Update documentation and examples

- Status: not started
- Description: Create comprehensive documentation and examples for the verification API, including usage patterns, error handling, and best practices.
- Acceptance Criteria:
  - Update `/c/README.md` with verification API documentation
  - Create `/c/examples/verify_signature.c` example program
  - Document all verification functions in header files
  - Include examples of:
    - Verifying signed messages
    - Verifying detached signatures
    - Error handling
    - Memory management
  - Add troubleshooting section for common issues
- Assumptions:
  - Following documentation style from previous phases
- Dependencies:
  - Tasks #3-8 (all implementation complete)
- Reference files:
  - `/c/README.md`
  - `/c/examples/` (existing examples)
  - `/docs/` (project documentation)
- Examples for implementing:
  - Good example:
    ```c
    /**
     * Verify a signed message and extract original content
     * 
     * @param signed_message The signed message in armored format
     * @param public_key The public key to verify against
     * @param result Output parameter for verification details
     * @return Operation result with error code
     * 
     * Example:
     *   openpgp_verification_result_t* result = NULL;
     *   openpgp_result_t res = openpgp_verify(signed_msg, pub_key, &result);
     *   if (res.error == OPENPGP_SUCCESS && result->is_valid) {
     *       printf("Original: %s\n", result->original_data);
     *   }
     *   openpgp_free_verification_result(result);
     */
    ```

### Task #10: Run comprehensive tests and fix issues

- Status: not started
- Description: Run all tests, fix any issues found, and ensure the implementation meets all requirements. This includes unit tests, integration tests, and memory leak testing.
- Acceptance Criteria:
  - All unit tests pass (`make c-test`)
  - All integration tests pass
  - No memory leaks detected by valgrind
  - Code coverage is adequate (aim for >90%)
  - Performance is acceptable (verification < 100ms for typical messages)
  - Update plan.md to mark Phase 7 as complete
  - Commit all changes with appropriate messages
- Assumptions:
  - Test infrastructure is working correctly
  - Valgrind is available for memory testing
- Dependencies:
  - Tasks #1-9 (all implementation and tests complete)
- Reference files:
  - `/c/test/` (all test files)
  - `/docs/plan.md`
- Examples for implementing:
  - Good testing checklist:
    - Run `make c-test` and verify all pass
    - Run `valgrind --leak-check=full ./test_verify`
    - Check code coverage with gcov
    - Test with large files (>1MB)
    - Test with various key types

## Test-Driven Development Approach

1. Write failing tests first (Task #2)
2. Implement minimal code to make tests pass (Tasks #3-6)
3. Refactor and optimize while keeping tests green
4. Add integration tests to verify interoperability (Task #7)
5. Ensure comprehensive test coverage

## Success Metrics

- All 4 verification functions implemented and working
- 100% of unit tests passing
- Integration tests verify compatibility with Go implementation
- No memory leaks detected
- Documentation is complete and examples work
- Phase 7 marked as complete in plan.md