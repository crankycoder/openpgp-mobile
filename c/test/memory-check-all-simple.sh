#!/bin/bash

# Simple Memory Check All - Run valgrind on all test executables
# Simplified version for Task #1 completion

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VALGRIND_RUNNER="$SCRIPT_DIR/valgrind-runner.sh"
BUILD_DIR="$SCRIPT_DIR/../build"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    cat << EOF
Usage: $0 [options]

Run valgrind memory checks on all available test executables.

Options:
  --help, -h         Show this help message
  --build-dir DIR    Directory containing test executables (default: $BUILD_DIR)
  --quiet           Suppress verbose output from individual tests

Examples:
  $0                              # Run all tests with default settings
  $0 --quiet                     # Run quietly
  $0 --build-dir /custom/build   # Use custom build directory
EOF
}

# Parse command line arguments
QUIET=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --quiet)
            QUIET="--quiet"
            shift
            ;;
        *)
            log_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Main execution
echo "=== Memory Check All - Simple Version ==="
echo "Build directory: $BUILD_DIR"
echo ""

# Check prerequisites
if [ ! -f "$VALGRIND_RUNNER" ]; then
    log_error "Valgrind runner not found: $VALGRIND_RUNNER"
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    log_error "Build directory not found: $BUILD_DIR"
    exit 1
fi

# Find test executables
test_executables=$(find "$BUILD_DIR" -name "test*" -type f -executable 2>/dev/null | sort)

if [ -z "$test_executables" ]; then
    log_error "No test executables found in $BUILD_DIR"
    exit 1
fi

log_info "Found $(echo "$test_executables" | wc -l) test executable(s)"
echo ""

# Run valgrind on each test
overall_exit_code=0
while IFS= read -r test_executable; do
    if [ -n "$test_executable" ]; then
        test_name=$(basename "$test_executable")
        log_info "Testing: $test_name"
        
        ((TOTAL_TESTS++))
        
        if "$VALGRIND_RUNNER" $QUIET "$test_executable"; then
            log_info "✓ PASSED: $test_name"
            ((PASSED_TESTS++))
        else
            log_error "✗ FAILED: $test_name"
            ((FAILED_TESTS++))
            overall_exit_code=1
        fi
        echo ""
    fi
done <<< "$test_executables"

# Summary
echo ""
log_info "=== Final Summary ==="
log_info "Total tests: $TOTAL_TESTS"
log_info "Passed: $PASSED_TESTS"
if [ "$FAILED_TESTS" -gt 0 ]; then
    log_error "Failed: $FAILED_TESTS"
    log_error "Memory issues detected in $FAILED_TESTS test(s)"
else
    log_info "Failed: $FAILED_TESTS"
    log_info "🎉 All tests passed! No memory issues detected."
fi

exit $overall_exit_code