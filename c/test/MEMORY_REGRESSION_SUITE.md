# Memory Regression Test Suite

## Overview

The Memory Regression Test Suite is a comprehensive collection of tests designed to prevent memory-related issues in the OpenPGP C library. This suite was developed as part of Phase 7.7: Comprehensive Valgrind Error Detection and Fixes to ensure all valgrind-detectable memory issues are prevented and regressions are caught early.

## Test Components

### 1. Error Path Testing (`test_memory_error_paths.c`)
- **Purpose**: Tests all error scenarios for proper memory cleanup
- **Coverage**: Invalid arguments, bridge call failures, oversized data handling
- **Key Features**:
  - Validates NULL parameter handling
  - Tests error propagation without memory leaks
  - Verifies proper cleanup on failure paths

### 2. Large Data Testing (`test_memory_large_data.c`)
- **Purpose**: Tests size limit boundary conditions for memory safety
- **Coverage**: FlatBuffer size limits, message boundaries, signature data limits
- **Key Features**:
  - Tests empirical 4KB FlatBuffer limit from Phase 7.6
  - Validates 2KB message size limit enforcement
  - Tests 3KB signature data size limit enforcement
  - Tests 512B key comment size limit enforcement

### 3. Stress Testing (`test_memory_stress.c`)
- **Purpose**: Tests repeated operations for memory leaks and stability
- **Coverage**: 1000+ iterations of operations, mixed operation patterns
- **Key Features**:
  - Detects memory accumulation over time
  - Tests rapid sequential operations
  - Validates memory stability under sustained load
  - Benchmarks performance under stress

### 4. Edge Cases Testing (`test_memory_edge_cases.c`)
- **Purpose**: Tests boundary conditions and unusual scenarios
- **Coverage**: NULL parameters, empty strings, special characters, integer overflow
- **Key Features**:
  - Tests NULL and empty string handling
  - Validates special character processing
  - Tests integer overflow scenarios
  - Verifies concurrent operation edge conditions### 5. Performance Benchmarks (`test_memory_performance.c`)
- **Purpose**: Benchmarks memory operations to detect performance regressions
- **Coverage**: Size validation performance, error handling overhead, allocation patterns
- **Key Features**:
  - Validates that memory fixes don't introduce significant overhead
  - Benchmarks size validation performance (< 100ms for 1000 operations)
  - Tests error handling speed (< 50ms for 1000 operations)
  - Monitors test isolation overhead

### 6. Comprehensive Test Runner (`test_memory_regression_suite.c`)
- **Purpose**: Unified test runner with reporting and valgrind integration
- **Coverage**: All test components with priority-based execution
- **Key Features**:
  - Runs all memory regression tests in sequence
  - Provides comprehensive reporting with timing
  - Integrates with valgrind for memory checking
  - Categorizes failures by priority (critical vs important)

## Test Infrastructure

### Test Isolation System
- **Global State Management**: Centralized reset of OpenPGP and memory tracking state
- **Thread-Safe Counters**: Prevents global variable conflicts between tests
- **Automatic Cleanup**: Ensures each test starts with clean state

### Size Validation Framework
- **Empirical Limits**: Based on Phase 7.6 FlatCC analysis
- **Early Validation**: Size checks before library initialization
- **Clear Error Messages**: Specific error codes for size limit violations

## Usage Instructions

### Running Individual Test Suites

```bash
# Compile test components
make test_memory_error_paths
make test_memory_large_data  
make test_memory_stress
make test_memory_edge_cases
make test_memory_performance

# Run individual test suites
./test_memory_error_paths
./test_memory_large_data
./test_memory_stress
./test_memory_edge_cases
./test_memory_performance
```

### Running Complete Regression Suite

```bash
# Compile and run comprehensive suite
make test_memory_regression_suite
./test_memory_regression_suite

# View help and valgrind instructions
./test_memory_regression_suite --help
./test_memory_regression_suite --valgrind-help
```

### Valgrind Integration

```bash
# Run individual tests with valgrind
valgrind --leak-check=full --show-leak-kinds=all ./test_memory_error_paths
valgrind --leak-check=full --show-leak-kinds=all ./test_memory_large_data
valgrind --leak-check=full --show-leak-kinds=all ./test_memory_stress
valgrind --leak-check=full --show-leak-kinds=all ./test_memory_edge_cases

# Run complete suite with valgrind
valgrind --leak-check=full --show-leak-kinds=all ./test_memory_regression_suite
```### CI Integration

```bash
# Add to CI pipeline with error exit codes
valgrind --leak-check=full --error-exitcode=1 ./test_memory_regression_suite

# Expected successful valgrind output:
# All heap blocks were freed -- no leaks are possible
# ERROR SUMMARY: 0 errors from 0 contexts
```

## Expected Test Results

### Success Criteria
- **All heap blocks freed**: No memory leaks detected
- **Zero valgrind errors**: No memory access violations
- **All size limits enforced**: Proper rejection of oversized data
- **Performance within bounds**: No significant overhead from memory fixes
- **Stable under stress**: No memory accumulation over repeated operations

### Failure Indicators
- Memory leaks detected by valgrind
- Invalid memory access (read/write after free, buffer overflows)
- Size limit bypasses allowing oversized data
- Performance regression (>100ms for 1000 basic operations)
- Memory accumulation during stress testing

## Maintenance Guidelines

### Adding New Tests
1. Follow the existing test file structure
2. Use the isolated test framework (`test_framework_isolated.h`)
3. Include test in the comprehensive runner
4. Document test purpose and coverage

### Updating Size Limits
1. Update constants in `openpgp.c` based on empirical analysis
2. Update corresponding test boundary values
3. Update documentation with new limits
4. Verify all tests pass with new limits

### Performance Monitoring
1. Run benchmarks regularly to detect regressions
2. Update performance thresholds if hardware changes
3. Monitor CI build times for performance impact
4. Profile with valgrind --tool=callgrind for detailed analysis

## Integration with Phase 7.7

This test suite is the culmination of Phase 7.7: Comprehensive Valgrind Error Detection and Fixes, which included:

1. **Task #1**: Fixed missing return statement (completed in 7.6)
2. **Task #2**: Fixed potential data access issues (completed in 7.6) 
3. **Task #3**: Fixed memory management issues (completed in 7.6)
4. **Task #4**: Fixed buffer management in serialization (completed in 7.6)
5. **Task #5**: Added CI integration for valgrind (completed in 7.6)
6. **Task #6**: Implemented test isolation system
7. **Task #7**: Added comprehensive size validation
8. **Task #8**: Created this Memory Regression Suite (current)
9. **Task #9**: Document memory management best practices (pending)

The test suite ensures that all fixes remain effective and prevents future regressions.