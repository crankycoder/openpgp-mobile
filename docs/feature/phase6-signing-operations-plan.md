# Phase 6: Signing Operations - Task Plan

## Overview
This plan details the implementation of signing operations for the OpenPGP Mobile C binding library. The signing functionality will allow users to create digital signatures for messages, files, and arbitrary data using their private keys.

## Architecture Considerations
- Continue using FlatBuffer serialization exclusively (no JSON)
- Bridge calls will use `OpenPGPBridgeCall("sign", buffer, size)` and similar operations
- Support multiple signature formats: cleartext, detached, and armored
- Follow the established pattern from previous phases

## Task Breakdown

### Task #1: Analyze FlatBuffer Schema for Signing Operations

- Status: not started
- Description: Examine the bridge.fbs file to understand the exact message structures for signing operations, including request and response types, available options, and signature formats.
- Acceptance Criteria:
  - Document all signing-related message types in bridge.fbs
  - Identify required and optional fields for SignRequest
  - Understand SignResponse structure and error handling
  - Note any signature format options (cleartext, detached, etc.)
- Assumptions:
  - bridge.fbs contains SignRequest and SignResponse message types
  - Signature format options are defined as enums
- Dependencies:
  - Access to flatbuffers/bridge.fbs file
- Reference files:
  - flatbuffers/bridge.fbs
  - c/generated/bridge_builder.h
  - c/generated/bridge_reader.h
- Examples for implementing:
  - Good example: Follow Phase 4's pattern of checking .fbs first before implementation
  - Bad example: Assuming message structure without verification

### Task #2: Define C API for Signing Functions

- Status: not started
- Description: Add signing function declarations and supporting structures to the public C API header file, ensuring consistency with existing API patterns.
- Acceptance Criteria:
  - Add openpgp_sign() function declaration
  - Add openpgp_sign_data() function declaration
  - Add openpgp_sign_file() function declaration
  - Add openpgp_sign_bytes() function declaration
  - Define signature format enum (CLEARTEXT, DETACHED, ARMORED)
  - Define openpgp_signature_t structure for results
  - Add openpgp_free_signature() for memory cleanup
  - Document all functions with clear comments
- Assumptions:
  - Signature operations require private key and optional passphrase
  - Different output formats are supported
- Dependencies:
  - Task #1 completion (understanding FlatBuffer structures)
- Reference files:
  - c/include/openpgp.h
  - c/src/openpgp.c (for existing patterns)
- Examples for implementing:
  - Good example:
    ```c
    typedef struct {
        char* signature;
        char* signed_data;  // For cleartext signatures
        openpgp_signature_format_t format;
    } openpgp_signature_t;
    
    openpgp_result_t openpgp_sign(
        const char* message,
        const char* private_key,
        const char* passphrase,
        openpgp_signature_format_t format
    );
    ```
  - Bad example: Using void* for signature data instead of proper structure

### Task #3: Create Sign Test File and Basic Tests

- Status: not started
- Description: Create the test file for signing operations and implement basic signing tests using Unity framework, following the testing patterns from previous phases.
- Acceptance Criteria:
  - Create c/test/test_sign.c file
  - Include Unity test framework headers
  - Import test keys from test_keys.h
  - Write test_sign_message_cleartext() test
  - Write test_sign_message_detached() test
  - Write test_sign_with_passphrase() test
  - All tests use TEST_ASSERT macros appropriately
- Assumptions:
  - Test keys from previous phases can be reused
  - Unity framework is properly configured
- Dependencies:
  - None (can be done in parallel with implementation)
- Reference files:
  - c/test/test_encrypt.c (for test patterns)
  - c/test/test_keys.h
  - c/test/unity.h
- Examples for implementing:
  - Good example:
    ```c
    void test_sign_message_cleartext(void) {
        const char* message = "Hello, World!";
        openpgp_result_t result = openpgp_sign(
            message,
            test_private_key_alice,
            NULL,
            OPENPGP_SIGNATURE_CLEARTEXT
        );
        TEST_ASSERT_EQUAL(OPENPGP_SUCCESS, result.error);
        TEST_ASSERT_NOT_NULL(result.signature);
        TEST_ASSERT_NOT_NULL(result.signature->signature);
        // Verify signature contains message in cleartext
        TEST_ASSERT_NOT_NULL(strstr(result.signature->signed_data, message));
    }
    ```
  - Bad example: Not checking for proper signature format in output

### Task #4: Implement FlatBuffer Serialization for Sign Operations

- Status: not started
- Description: Implement the core signing functionality in openpgp.c, including FlatBuffer serialization for sign requests and response parsing.
- Acceptance Criteria:
  - Implement openpgp_sign() function
  - Create FlatBuffer builder and build SignRequest
  - Handle different signature formats in request
  - Call OpenPGPBridgeCall("sign", buffer, size)
  - Parse SignResponse FlatBuffer
  - Extract signature data based on format
  - Implement proper error handling
  - Free all allocated memory correctly
- Assumptions:
  - SignRequest includes fields for data, private_key, passphrase, and format
  - SignResponse contains signature field and possibly error
- Dependencies:
  - Task #1 completion (FlatBuffer schema understanding)
  - Task #2 completion (API definitions)
- Reference files:
  - c/src/openpgp.c
  - c/generated/bridge_builder.h
  - c/generated/bridge_reader.h
- Examples for implementing:
  - Good example: Use manual FlatBuffer building pattern from Phase 1
  - Bad example: Using create() functions that fail with optional fields

### Task #5: Implement Sign Data and Sign Bytes Functions

- Status: not started
- Description: Implement the variations of signing functions for raw data and byte arrays, ensuring consistent behavior and proper memory handling.
- Acceptance Criteria:
  - Implement openpgp_sign_data() function
  - Implement openpgp_sign_bytes() function
  - Handle binary data correctly (not null-terminated)
  - Support all signature formats for binary data
  - Add appropriate tests for binary signing
  - Ensure no data corruption for non-text input
- Assumptions:
  - Binary data may contain null bytes
  - Size parameter is required for binary operations
- Dependencies:
  - Task #4 completion (core signing implementation)
- Reference files:
  - c/src/openpgp.c
  - c/test/test_symmetric.c (for binary data handling patterns)
- Examples for implementing:
  - Good example: Pass explicit length for binary data
  - Bad example: Using strlen() on binary data

### Task #6: Implement Sign File Function

- Status: not started
- Description: Implement file signing functionality that reads file content and produces appropriate signatures, with support for large files.
- Acceptance Criteria:
  - Implement openpgp_sign_file() function
  - Read file content safely
  - Handle large files appropriately
  - Support all signature formats
  - Create detached signature files when requested
  - Add file signing tests
  - Handle file read errors gracefully
- Assumptions:
  - File paths are provided as absolute paths
  - Detached signatures save to filename.sig
- Dependencies:
  - Task #4 completion (core signing implementation)
- Reference files:
  - c/src/openpgp.c
  - Standard C file I/O patterns
- Examples for implementing:
  - Good example: Check file exists before reading
  - Bad example: Loading entire large file into memory

### Task #7: Add Advanced Signing Tests

- Status: not started
- Description: Create comprehensive tests for edge cases, error conditions, and compatibility with Go implementation signatures.
- Acceptance Criteria:
  - Test signing with wrong passphrase
  - Test signing with invalid private key
  - Test empty message signing
  - Test large message signing
  - Test special characters in messages
  - Import and verify Go-created signatures
  - Test memory management with valgrind
- Assumptions:
  - Go test suite has example signatures we can verify
- Dependencies:
  - Tasks #3-6 completion
- Reference files:
  - c/test/test_sign.c
  - Go test files for signature examples
- Examples for implementing:
  - Good example: Comprehensive error condition testing
  - Bad example: Only testing happy path

### Task #8: Implement Signature Memory Management

- Status: not started
- Description: Ensure proper memory allocation and deallocation for all signature operations, preventing memory leaks.
- Acceptance Criteria:
  - Implement openpgp_free_signature() function
  - All allocated strings are properly freed
  - Signature structure is fully cleaned up
  - No memory leaks detected by valgrind
  - Add memory management tests
- Assumptions:
  - Users are responsible for calling free function
- Dependencies:
  - Task #4 completion
- Reference files:
  - c/src/openpgp.c
  - c/include/openpgp.h
- Examples for implementing:
  - Good example: Free both signature and signed_data fields
  - Bad example: Only freeing the structure, not internal pointers

### Task #9: Create Signing Documentation and Examples

- Status: not started
- Description: Document the signing API with clear examples showing different use cases and signature formats.
- Acceptance Criteria:
  - Add signing section to C API documentation
  - Create example: sign_message.c
  - Create example: sign_file.c
  - Create example: verify_signature.c (placeholder for Phase 7)
  - Document each signature format and when to use it
  - Include memory management best practices
  - Show error handling patterns
- Assumptions:
  - Documentation follows existing format
- Dependencies:
  - Tasks #1-8 completion
- Reference files:
  - c/examples/
  - docs/c-api.md (if exists)
- Examples for implementing:
  - Good example: Complete working examples with error handling
  - Bad example: Code snippets without context

### Task #10: Integration Testing and Validation

- Status: not started
- Description: Perform final integration testing to ensure signing operations work correctly with all other implemented features.
- Acceptance Criteria:
  - All signing tests pass (make c-test)
  - Can sign keys generated in Phase 5
  - Signatures are valid OpenPGP format
  - No memory leaks in any signing operation
  - Performance is acceptable (<1s for typical operations)
  - Create PR-ready summary of changes
- Assumptions:
  - All previous phases are working correctly
- Dependencies:
  - Tasks #1-9 completion
- Reference files:
  - All test files
  - Makefile
- Examples for implementing:
  - Good example: Run full test suite after implementation
  - Bad example: Testing only new functionality in isolation

## Risk Mitigation

1. **FlatBuffer Complexity**: Based on Phase 1 learnings, use manual building pattern for optional fields
2. **Memory Management**: Follow established patterns from previous phases
3. **Signature Format Compatibility**: Test against known good signatures from Go implementation
4. **Large File Handling**: Implement streaming for file operations if needed

## Success Metrics

- All 10 tasks completed
- 100% test coverage for signing operations
- No memory leaks detected
- Signatures compatible with Go implementation
- Clear documentation and examples provided
- Performance within acceptable limits

## Commit Strategy

Each task should result in one or more atomic commits:
- Task #1: Analysis only, may not require commit
- Task #2: `feat(c-binding): add signing API declarations`
- Task #3: `test(signing): add basic signing test structure`
- Task #4: `feat(c-binding): implement core signing functionality`
- Task #5: `feat(c-binding): add binary data signing support`
- Task #6: `feat(c-binding): implement file signing operations`
- Task #7: `test(signing): add comprehensive signing tests`
- Task #8: `fix(signing): implement proper memory management`
- Task #9: `docs(signing): add documentation and examples`
- Task #10: `test(signing): complete integration testing`