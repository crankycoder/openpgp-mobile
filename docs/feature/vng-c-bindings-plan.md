# Task Plan: Phase 7.8 - Valgrind Memory Leak Fixes

## Current Status
- Branch: `vng/c-bindings`
- Phase: 7.8 (Memory Leak Fixes)
- Status: IN PROGRESS

## Objective
Fix all remaining valgrind-detected memory leaks across the C test suite to ensure memory safety and establish patterns for future development.

## Task Breakdown

### Task 1: Verify test_memory_large_data.c Fixes
**Priority**: High  
**Status**: Ready to start  
**Dependencies**: None  

**Steps**:
1. Run valgrind on test_memory_large_data to verify fixes
2. Address any remaining leaks
3. Commit changes after verification

**Test Command**:
```bash
make memory-test-valgrind-single TEST=test_memory_large_data
```

### Task 2: Fix test_memory_stress.c Memory Leaks
**Priority**: High  
**Status**: Not started  
**Dependencies**: Task 1 completion  
**Current Issues**: 29KB leaked (batch operations)

**Steps**:
1. Add CHECK_RESULT_AND_FREE macro to file
2. Fix batch operation memory leaks in test_large_batch_operations
3. Fix repeated operation leaks
4. Run valgrind to verify
5. Commit changes

**Known Issues**:
- Batch operations allocating arrays not being freed
- Error messages from repeated operations not cleaned up

### Task 3: Fix test_memory_edge_cases.c Memory Leaks  
**Priority**: High  
**Status**: Not started  
**Dependencies**: Task 2 completion  
**Current Issues**: 1.2KB leaked

**Steps**:
1. Add CHECK_RESULT_AND_FREE macro
2. Update all test cases to free error messages before assertions
3. Fix test data allocation patterns
4. Run valgrind to verify
5. Commit changes

### Task 4: Fix test_memory_performance.c Memory Leaks
**Priority**: High  
**Status**: Not started  
**Dependencies**: Task 3 completion  
**Current Issues**: 67KB leaked (benchmark loops)

**Steps**:
1. Add CHECK_RESULT_AND_FREE macro
2. Fix benchmark loop memory management
3. Ensure error messages freed in performance tests
4. Run valgrind to verify  
5. Commit changes

**Special Considerations**:
- Performance tests run many iterations - need careful cleanup

### Task 5: Create Memory Testing Guidelines
**Priority**: Medium  
**Status**: Not started  
**Dependencies**: Tasks 1-4 complete  

**Steps**:
1. Create docs/c-memory-testing-guidelines.md
2. Document CHECK_RESULT_AND_FREE pattern
3. Provide examples of proper test memory management
4. Add section on common pitfalls
5. Commit documentation

### Task 6: Run Full Valgrind Suite
**Priority**: High  
**Status**: Not started  
**Dependencies**: Task 5 complete  

**Steps**:
1. Run complete memory test suite under valgrind
2. Verify all tests pass with 0 leaks
3. Update plan.md to mark Phase 7.8 complete
4. Final commit

## Test-Driven Development Approach

For each task:
1. Run valgrind first to see current state
2. Make minimal changes to fix identified leaks
3. Re-run valgrind to verify fix
4. Commit immediately after verification

## Git Workflow Reminders

- We're on feature branch `vng/c-bindings`
- Commit after each task completion
- Use semantic commit messages (fix: for leak fixes)
- Run tests before every commit

## Success Criteria

- All memory tests pass valgrind with 0 leaks
- CHECK_RESULT_AND_FREE pattern used consistently
- Documentation created for future developers
- No regression in functionality

## Commands Reference

```bash
# Build memory tests
make memory-tests

# Run specific test under valgrind
make memory-test-valgrind-single TEST=test_name

# Run all memory tests under valgrind  
make memory-test-valgrind

# Run regular C tests
make c-test
```