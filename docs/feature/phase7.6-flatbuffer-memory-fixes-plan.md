# Phase 7.6: FlatBuffer Memory Leak Debugging - Task Plan

## Overview
This plan implements Phase 7.6 from the main project plan using Test-Driven Development (TDD) to systematically identify and fix FlatBuffer memory leaks detected by valgrind.

## Known Issues from Valgrind Analysis
1. Invalid read/write errors in FlatBuffer serialization
2. Buffer overruns in `serialize_generate_request()`
3. Uninitialized values in `libopenpgp_bridge.so`
4. Memory corruption causing delayed segfaults in test suite
5. Test interference due to corrupted memory state

## TDD Strategy
Each test will be written to fail first, then implementation will be fixed to make it pass.

---

## Task List

### Task #1: Create FlatBuffer Memory Test Suite Infrastructure

- Status: not started
- Description: Create isolated test suite specifically for FlatBuffer memory management with valgrind integration
- Acceptance Criteria:
  - Test suite can run under valgrind without false positives
  - Each test is completely isolated (separate process if needed)
  - Memory tracking shows allocations and deallocations
  - Valgrind output is captured and analyzed automatically
  - Test infrastructure is reusable for all memory tests
- Assumptions:
  - Unity test framework is already available
  - Valgrind is installed on the development system
  - flatcc runtime is available from `make flatbuffers_c`
- Dependencies:
  - None
- Reference files:
  - `/c/test/unity.h` - Test framework
  - `/c/test/test_runner.c` - Test runner infrastructure
- Examples for implementing:
  - Good example: Create memory tracking wrapper functions that log allocations
  - Bad example: Relying on manual memory tracking without automation

### Task #2: Test Basic FlatBuffer Builder Lifecycle

- Status: not started
- Description: Test the fundamental builder create/destroy cycle to establish baseline memory behavior
- Acceptance Criteria:
  - Test passes under valgrind with zero memory leaks
  - Builder creation and destruction is properly tracked
  - No invalid reads or writes detected
  - Clear documentation of proper usage pattern
- Assumptions:
  - FlatBuffer builder API is available through generated headers
  - Basic builder operations should not leak memory
- Dependencies:
  - Task #1 must be complete
- Reference files:
  - `/c/generated/bridge_builder.h` - FlatBuffer builder API
  - `/c/src/openpgp.c` - Current builder usage patterns
- Examples for implementing:
  - Good example: Test both immediate destroy and destroy after buffer operations
  - Bad example: Only testing the happy path without edge cases

### Task #3: Fix serialize_generate_request Buffer Management

- Status: not started
- Description: Fix buffer overrun in serialize_generate_request using TDD approach
- Acceptance Criteria:
  - No buffer overruns with any input size
  - Proper error handling for oversized inputs
  - Valgrind reports no invalid reads/writes
  - Buffer size calculations are documented
- Assumptions:
  - Buffer overrun is due to incorrect size calculation
  - FlatBuffer provides size estimation APIs
- Dependencies:
  - Task #2 must be complete
- Reference files:
  - `/c/src/openpgp.c` - serialize_generate_request function
  - `/c/test/test_generate.c` - Existing generation tests
- Examples for implementing:
  - Good example: Calculate exact buffer size before allocation
  - Bad example: Using fixed buffer sizes without validation

### Task #4: Test FlatBuffer String Handling

- Status: not started
- Description: Test and fix string serialization issues which are common sources of buffer problems
- Acceptance Criteria:
  - NULL strings handled correctly
  - Empty strings handled correctly
  - Long strings (>1KB) handled without overflow
  - UTF-8 strings handled properly
  - No memory leaks with any string input
- Assumptions:
  - String handling is a major source of memory issues
  - FlatBuffer has specific string handling requirements
- Dependencies:
  - Task #3 must be complete
- Reference files:
  - `/c/src/openpgp.c` - String serialization code
  - `/c/generated/bridge_builder.h` - String builder APIs
- Examples for implementing:
  - Good example: Test boundary conditions (NULL, "", very long)
  - Bad example: Only testing ASCII strings of normal length

### Task #5: Test Nested FlatBuffer Structures

- Status: not started
- Description: Test complex nested structures (Options, KeyOptions, etc.) for memory leaks
- Acceptance Criteria:
  - Complex structures serialize without leaks
  - Optional fields handled correctly
  - Nested structures properly cleaned up
  - No accumulating memory usage
- Assumptions:
  - Nested structures may have different memory management requirements
  - Optional fields need special handling
- Dependencies:
  - Task #4 must be complete
- Reference files:
  - `/flatbuffers/bridge.fbs` - Schema definitions
  - `/c/src/openpgp.c` - Nested structure usage
- Examples for implementing:
  - Good example: Test with all combinations of optional fields
  - Bad example: Only testing fully populated structures

### Task #6: Test FlatBuffer Response Parsing Memory

- Status: not started
- Description: Test deserialization to ensure it doesn't leak memory
- Acceptance Criteria:
  - Response parsing has zero leaks
  - Error paths properly clean up
  - Multiple parse operations don't accumulate memory
  - Malformed responses handled gracefully
- Assumptions:
  - Response parsing may allocate temporary buffers
  - Error handling paths may leak memory
- Dependencies:
  - Task #5 must be complete
- Reference files:
  - `/c/src/openpgp.c` - Response parsing functions
  - `/c/generated/bridge_reader.h` - Reader APIs
- Examples for implementing:
  - Good example: Test both valid and malformed responses
  - Bad example: Only testing successful parsing

### Task #7: Fix Cross-Test Memory Isolation

- Status: not started
- Description: Fix test interference issues by ensuring proper cleanup between tests
- Acceptance Criteria:
  - Tests don't interfere with each other
  - Clear isolation boundaries
  - No state leakage between operations
  - Test order doesn't affect results
- Assumptions:
  - Current tests may share state inadvertently
  - Memory corruption affects subsequent tests
- Dependencies:
  - Task #6 must be complete
- Reference files:
  - `/c/test/test_runner.c` - Test execution framework
  - All test files in `/c/test/`
- Examples for implementing:
  - Good example: Reset all global state between tests
  - Bad example: Assuming tests start with clean state

### Task #8: Fix flatcc_builder_end_buffer Corruption

- Status: not started
- Description: Address specific corruption in end_buffer operations
- Acceptance Criteria:
  - No corruption in end_buffer
  - Clear error handling
  - Double-end operations handled gracefully
  - Builder state validated before operations
- Assumptions:
  - Corruption may be due to invalid builder state
  - Double-free or use-after-free may be occurring
- Dependencies:
  - Task #7 must be complete
- Reference files:
  - `/c/src/openpgp.c` - All end_buffer usage
  - flatcc documentation (if available)
- Examples for implementing:
  - Good example: Validate builder state before every operation
  - Bad example: Assuming builder is always in valid state

### Task #9: Create Memory Regression Test Suite

- Status: not started
- Description: Comprehensive tests to prevent regressions
- Acceptance Criteria:
  - All operations pass valgrind checks
  - CI automatically runs memory tests
  - Clear memory usage documentation
  - Performance impact < 5%
- Assumptions:
  - Regression tests will catch future memory issues
  - CI can be configured to run valgrind
- Dependencies:
  - Tasks #1-8 must be complete
- Reference files:
  - All test files
  - `/c/Makefile` - Build system
- Examples for implementing:
  - Good example: Automated valgrind runs with clear pass/fail
  - Bad example: Manual memory testing only

---

## Implementation Notes

1. **TDD Process for Each Task**:
   - Write failing test that exposes the memory issue
   - Run under valgrind to confirm the issue
   - Implement minimal fix to pass the test
   - Refactor if needed while keeping tests green
   - Document the fix and root cause

2. **Valgrind Configuration**:
   ```bash
   valgrind --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --error-exitcode=1 \
            --suppressions=valgrind.supp \
            ./test_program
   ```

3. **Commit Strategy**:
   - Commit after each test file creation
   - Commit after each test implementation
   - Commit after each fix implementation
   - Use semantic commit format

4. **Success Metrics**:
   - Zero valgrind errors in all tests
   - No memory leaks detected
   - No buffer overruns or underruns
   - Clear memory management documentation