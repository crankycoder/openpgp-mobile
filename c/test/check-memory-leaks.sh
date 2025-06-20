#!/bin/bash

# Automated Memory Leak Prevention Script
# Runs valgrind on all memory regression test files to prevent regressions

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
C_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$C_DIR/build"
VALGRIND_RUNNER="$SCRIPT_DIR/valgrind-runner.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=== Automated Memory Leak Check ==="
echo "Checking all memory regression test files..."
echo ""

# List of memory test files to check
MEMORY_TESTS=(
    "test_memory_error_paths"
    "test_memory_large_data" 
    "test_memory_stress"
    "test_memory_edge_cases"
    "test_memory_performance"
)

failed_tests=()
passed_tests=()

# Run valgrind on each memory test
for test in "${MEMORY_TESTS[@]}"; do
    test_executable="$BUILD_DIR/$test"
    
    if [ ! -f "$test_executable" ]; then
        echo -e "${YELLOW}[SKIP]${NC} $test - executable not found"
        continue
    fi
    
    echo -e "Testing: ${YELLOW}$test${NC}"
    
    if "$VALGRIND_RUNNER" --quiet "$test_executable" > /dev/null 2>&1; then
        echo -e "${GREEN}[PASS]${NC} $test - no memory leaks detected"
        passed_tests+=("$test")
    else
        echo -e "${RED}[FAIL]${NC} $test - memory leaks detected"
        failed_tests+=("$test")
    fi
    echo ""
done

# Summary
echo "=== Memory Leak Check Summary ==="
echo "Tests passed: ${#passed_tests[@]}"
echo "Tests failed: ${#failed_tests[@]}"

if [ ${#passed_tests[@]} -gt 0 ]; then
    echo -e "${GREEN}Passed tests:${NC}"
    for test in "${passed_tests[@]}"; do
        echo "  ✓ $test"
    done
fi

if [ ${#failed_tests[@]} -gt 0 ]; then
    echo -e "${RED}Failed tests:${NC}"
    for test in "${failed_tests[@]}"; do
        echo "  ✗ $test"
    done
    echo ""
    echo -e "${RED}MEMORY LEAKS DETECTED!${NC}"
    echo "Run individual tests with valgrind-runner.sh for details"
    exit 1
fi

echo -e "${GREEN}All memory tests passed!${NC}"
exit 0