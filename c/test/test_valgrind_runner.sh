#!/bin/bash

# Test script for valgrind-runner.sh functionality
# This validates that the valgrind runner works correctly

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VALGRIND_RUNNER="$SCRIPT_DIR/valgrind-runner.sh"
TEST_PASSED=0
TEST_FAILED=0

log_info() {
    echo -e "${GREEN}[TEST INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[TEST ERROR]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[TEST WARNING]${NC} $1"
}

run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected_exit_code="${3:-0}"
    
    log_info "Running test: $test_name"
    
    if eval "$test_command" >/dev/null 2>&1; then
        local actual_exit_code=0
    else
        local actual_exit_code=$?
    fi
    
    if [ "$actual_exit_code" -eq "$expected_exit_code" ]; then
        log_info "✓ PASS: $test_name"
        ((TEST_PASSED++))
        return 0
    else
        log_error "✗ FAIL: $test_name (expected exit code $expected_exit_code, got $actual_exit_code)"
        ((TEST_FAILED++))
        return 1
    fi
}

# Test 1: valgrind-runner.sh exists and is executable
test_runner_exists() {
    if [ -f "$VALGRIND_RUNNER" ] && [ -x "$VALGRIND_RUNNER" ]; then
        return 0
    else
        return 1
    fi
}

# Test 2: valgrind-runner.sh shows help
test_runner_help() {
    "$VALGRIND_RUNNER" --help | grep -q "Usage"
}

# Test 3: valgrind-runner.sh fails gracefully with missing executable
test_runner_missing_executable() {
    "$VALGRIND_RUNNER" /nonexistent/test/executable
}

# Test 4: valgrind.supp exists
test_suppressions_exist() {
    [ -f "$SCRIPT_DIR/valgrind.supp" ]
}

# Test 5: memory-check-all.sh exists and is executable
test_memory_check_all_exists() {
    local memory_check_script="$SCRIPT_DIR/memory-check-all.sh"
    [ -f "$memory_check_script" ] && [ -x "$memory_check_script" ]
}

# Main test execution
main() {
    log_info "=== Valgrind Runner Test Suite ==="
    echo ""
    
    # Test that valgrind-runner.sh exists and is executable
    run_test "valgrind-runner.sh exists and is executable" "test_runner_exists"
    
    # Test help functionality
    run_test "valgrind-runner.sh shows help" "test_runner_help"
    
    # Test error handling
    run_test "valgrind-runner.sh handles missing executable" "test_runner_missing_executable" 1
    
    # Test suppressions file exists
    run_test "valgrind.supp exists" "test_suppressions_exist"
    
    # Test memory-check-all.sh exists
    run_test "memory-check-all.sh exists and is executable" "test_memory_check_all_exists"
    
    echo ""
    log_info "=== Test Results ==="
    log_info "Tests passed: $TEST_PASSED"
    if [ "$TEST_FAILED" -gt 0 ]; then
        log_error "Tests failed: $TEST_FAILED"
        log_error "Valgrind runner infrastructure is not ready"
        return 1
    else
        log_info "All tests passed! Valgrind runner infrastructure is ready"
        return 0
    fi
}

# Run the tests
main "$@"