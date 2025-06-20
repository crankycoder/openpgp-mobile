#!/bin/bash

# Memory Check All - Run valgrind on all test executables
# This script discovers and runs valgrind on all available test executables

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VALGRIND_RUNNER="$SCRIPT_DIR/valgrind-runner.sh"
BUILD_DIR="$SCRIPT_DIR/../build"
LOG_DIR="$SCRIPT_DIR/valgrind_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
REPORT_FILE="$SCRIPT_DIR/valgrind-report-${TIMESTAMP}.md"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_section() {
    echo -e "${BLUE}[SECTION]${NC} $1"
}

show_help() {
    cat << EOF
Usage: $0 [options]

Run valgrind memory checks on all available test executables.

Options:
  --help, -h         Show this help message
  --build-dir DIR    Directory containing test executables (default: $BUILD_DIR)
  --clean            Clean log directory before running
  --quiet           Suppress verbose output from individual tests
  --report-only     Generate report from existing logs without running tests

Examples:
  $0                              # Run all tests with default settings  $0 --clean --quiet             # Clean logs and run quietly
  $0 --build-dir /custom/build   # Use custom build directory
  $0 --report-only               # Just generate report from existing logs

Exit codes:
  0   All tests passed
  1   Some tests failed or other error occurred
EOF
}

discover_test_executables() {
    local build_dir="$1"
    
    log_info "Discovering test executables in: $build_dir"
    
    if [ ! -d "$build_dir" ]; then
        log_error "Build directory not found: $build_dir"
        log_info "Please build the tests first with: make c-test"
        return 1
    fi
    
    # Find all test executables (files starting with "test" and executable)
    local test_files=$(find "$build_dir" -name "test*" -type f -executable 2>/dev/null | sort)
    
    if [ -z "$test_files" ]; then
        log_warning "No test executables found in $build_dir"
        log_info "Available files:"
        ls -la "$build_dir" 2>/dev/null || echo "Directory empty or not accessible"
        return 1
    fi
    
    echo "$test_files"
    return 0
}

run_single_test() {
    local test_executable="$1"
    local test_name=$(basename "$test_executable")
    
    log_section "Running valgrind on: $test_name"
    
    ((TOTAL_TESTS++))
    
    local runner_args=""
    if [ "$QUIET" = "1" ]; then
        runner_args="--quiet"
    fi
    
    if "$VALGRIND_RUNNER" $runner_args "$test_executable"; then
        log_info "✓ PASSED: $test_name"
        ((PASSED_TESTS++))
        return 0
    else
        log_error "✗ FAILED: $test_name"
        ((FAILED_TESTS++))
        return 1
    fi
}generate_report() {
    log_info "Generating comprehensive report: $REPORT_FILE"
    
    cat > "$REPORT_FILE" << EOF
# Valgrind Memory Check Report

**Generated:** $(date)  
**Total Tests:** $TOTAL_TESTS  
**Passed:** $PASSED_TESTS  
**Failed:** $FAILED_TESTS  

## Summary

EOF

    if [ "$FAILED_TESTS" -eq 0 ]; then
        echo "✅ **All tests passed!** No memory issues detected." >> "$REPORT_FILE"
    else
        echo "❌ **$FAILED_TESTS test(s) failed** with memory issues." >> "$REPORT_FILE"
    fi
    
    cat >> "$REPORT_FILE" << EOF

## Test Results

| Test | Status | Memory Issues |
|------|--------|---------------|
EOF

    # Analyze each log file
    for log_file in "$LOG_DIR"/valgrind_*_${TIMESTAMP}.log; do
        if [ -f "$log_file" ]; then
            local test_name=$(basename "$log_file" | sed 's/valgrind_\(.*\)_[0-9_]*.log/\1/')
            local error_count=$(grep "ERROR SUMMARY:" "$log_file" | tail -1 | sed 's/.*ERROR SUMMARY: \([0-9]*\).*/\1/' 2>/dev/null || echo "0")
            local leak_count=$(grep -c "definitely lost\|indirectly lost\|possibly lost" "$log_file" 2>/dev/null || echo "0")
            
            local status="✅ PASS"
            local issues="None"
            
            if [ "${error_count:-0}" -gt 0 ] || [ "${leak_count:-0}" -gt 0 ]; then
                status="❌ FAIL"
                issues="${error_count:-0} errors, ${leak_count:-0} leaks"
            fi
            
            echo "| $test_name | $status | $issues |" >> "$REPORT_FILE"
        fi
    done
}    cat >> "$REPORT_FILE" << EOF

## Detailed Analysis

EOF

    # Add detailed analysis for failed tests
    for log_file in "$LOG_DIR"/valgrind_*_${TIMESTAMP}.log; do
        if [ -f "$log_file" ]; then
            local test_name=$(basename "$log_file" | sed 's/valgrind_\(.*\)_[0-9_]*.log/\1/')
            local error_count=$(grep "ERROR SUMMARY:" "$log_file" | tail -1 | sed 's/.*ERROR SUMMARY: \([0-9]*\).*/\1/' 2>/dev/null || echo "0")
            local leak_count=$(grep -c "definitely lost\|indirectly lost\|possibly lost" "$log_file" 2>/dev/null || echo "0")
            
            if [ "${error_count:-0}" -gt 0 ] || [ "${leak_count:-0}" -gt 0 ]; then
                cat >> "$REPORT_FILE" << EOF

### $test_name

**Issues found:** ${error_count:-0} errors, ${leak_count:-0} memory leaks

**Log file:** $log_file

**Error summary:**
\`\`\`
$(grep "ERROR SUMMARY:" "$log_file" | tail -1)
\`\`\`

EOF
                if [ "${leak_count:-0}" -gt 0 ]; then
                    echo "**Memory leaks:**" >> "$REPORT_FILE"
                    echo "\`\`\`" >> "$REPORT_FILE"
                    grep -A 5 -B 1 "definitely lost\|indirectly lost\|possibly lost" "$log_file" | head -20 >> "$REPORT_FILE"
                    echo "\`\`\`" >> "$REPORT_FILE"
                    echo "" >> "$REPORT_FILE"
                fi
            fi
        fi
    done
    
    log_info "Report generated successfully!"
    log_info "View report: $REPORT_FILE"
}

# Parse command line arguments
QUIET=0
CLEAN=0
REPORT_ONLY=0

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
        --clean)
            CLEAN=1
            shift
            ;;
        --quiet)
            QUIET=1
            shift
            ;;
        --report-only)
            REPORT_ONLY=1
            shift
            ;;
        -*)
            log_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
        *)
            log_error "Unexpected argument: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done# Main execution
main() {
    echo "=== Memory Check All - Valgrind Test Suite ==="
    echo "Build directory: $BUILD_DIR"
    echo "Timestamp: $TIMESTAMP"
    echo ""
    
    # Check prerequisites
    if [ ! -f "$VALGRIND_RUNNER" ]; then
        log_error "Valgrind runner not found: $VALGRIND_RUNNER"
        exit 1
    fi
    
    # Clean logs if requested
    if [ "$CLEAN" = "1" ]; then
        log_info "Cleaning log directory: $LOG_DIR"
        rm -rf "$LOG_DIR"
        mkdir -p "$LOG_DIR"
    fi
    
    if [ "$REPORT_ONLY" = "1" ]; then
        log_info "Generating report from existing logs..."
        generate_report
        return $?
    fi
    
    # Discover test executables
    local test_executables
    if ! test_executables=$(discover_test_executables "$BUILD_DIR"); then
        exit 1
    fi
    
    log_info "Found $(echo "$test_executables" | wc -l) test executable(s)"
    echo ""
    
    # Run valgrind on each test
    local overall_exit_code=0
    while IFS= read -r test_executable; do
        if [ -n "$test_executable" ]; then
            run_single_test "$test_executable" || overall_exit_code=1
            echo ""
        fi
    done <<< "$test_executables"
    
    # Generate comprehensive report
    generate_report
    
    # Summary
    echo ""
    log_info "=== Final Summary ==="
    log_info "Total tests: $TOTAL_TESTS"
    log_info "Passed: $PASSED_TESTS"
    if [ "$FAILED_TESTS" -gt 0 ]; then
        log_error "Failed: $FAILED_TESTS"
        log_error "Memory issues detected in $FAILED_TESTS test(s)"
        overall_exit_code=1
    else
        log_info "Failed: $FAILED_TESTS"
        log_info "🎉 All tests passed! No memory issues detected."
    fi
    
    exit $overall_exit_code
}

# Run main function
main "$@"