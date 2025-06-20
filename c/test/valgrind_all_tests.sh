#!/bin/bash

# Comprehensive Valgrind Test Runner for All C Tests
# This script runs each test executable individually under valgrind
# to detect and diagnose all memory issues

set -e

# Configuration
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 --verbose"
VALGRIND_SUPPRESSIONS=""
BUILD_DIR="../build"
LOG_DIR="./valgrind_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
SUMMARY_FILE="$LOG_DIR/valgrind_summary_${TIMESTAMP}.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results tracking
declare -A test_results
declare -A test_issues
total_tests=0
passed_tests=0
failed_tests=0

# Functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_test() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

check_prerequisites() {
    log_info "Checking prerequisites..."
    
    # Check if valgrind is available
    if ! command -v valgrind &> /dev/null; then
        log_error "Valgrind is not installed or not in PATH"
        exit 1
    fi
    
    # Check if build directory exists
    if [ ! -d "$BUILD_DIR" ]; then
        log_error "Build directory not found: $BUILD_DIR"
        log_info "Please build the tests first with: make c-test"
        exit 1
    fi
    
    # Create log directory
    mkdir -p "$LOG_DIR"
    
    # Check if suppressions file exists
    if [ -f "valgrind.supp" ]; then
        VALGRIND_SUPPRESSIONS="--suppressions=valgrind.supp"
        log_info "Using suppressions file: valgrind.supp"
    fi
    
    log_info "Prerequisites check passed"
}

find_test_executables() {
    log_info "Finding test executables in $BUILD_DIR..."
    
    # Find all test executables (assuming they start with 'test_')
    local test_files=$(find "$BUILD_DIR" -type f -executable -name "test_*" | sort)
    
    if [ -z "$test_files" ]; then
        log_error "No test executables found in $BUILD_DIR"
        exit 1
    fi
    
    echo "$test_files"
}

run_single_test_valgrind() {
    local test_executable="$1"
    local test_name=$(basename "$test_executable")
    local log_file="$LOG_DIR/valgrind_${test_name}_${TIMESTAMP}.log"
    
    log_test "Running: $test_name"
    
    # Run valgrind
    local cmd="valgrind $VALGRIND_OPTS $VALGRIND_SUPPRESSIONS $test_executable"
    
    echo "Command: $cmd" > "$log_file"
    echo "Timestamp: $(date)" >> "$log_file"
    echo "========================" >> "$log_file"
    
    # Run the test and capture exit code
    local exit_code=0
    if $cmd >> "$log_file" 2>&1; then
        exit_code=0
    else
        exit_code=$?
    fi
    
    # Analyze the log for memory issues
    local memory_issues=""
    local definitely_lost=$(grep -oP "definitely lost: \K[0-9,]+" "$log_file" | tail -1 || echo "0")
    local indirectly_lost=$(grep -oP "indirectly lost: \K[0-9,]+" "$log_file" | tail -1 || echo "0")
    local possibly_lost=$(grep -oP "possibly lost: \K[0-9,]+" "$log_file" | tail -1 || echo "0")
    local still_reachable=$(grep -oP "still reachable: \K[0-9,]+" "$log_file" | tail -1 || echo "0")
    local error_count=$(grep -oP "ERROR SUMMARY: \K[0-9]+" "$log_file" | tail -1 || echo "0")
    
    # Remove commas from numbers for comparison
    definitely_lost=${definitely_lost//,/}
    indirectly_lost=${indirectly_lost//,/}
    possibly_lost=${possibly_lost//,/}
    still_reachable=${still_reachable//,/}
    
    # Check for issues
    if [ "$definitely_lost" != "0" ]; then
        memory_issues="${memory_issues}Definitely lost: $definitely_lost bytes. "
    fi
    if [ "$indirectly_lost" != "0" ]; then
        memory_issues="${memory_issues}Indirectly lost: $indirectly_lost bytes. "
    fi
    if [ "$possibly_lost" != "0" ]; then
        memory_issues="${memory_issues}Possibly lost: $possibly_lost bytes. "
    fi
    if [ "$error_count" != "0" ]; then
        memory_issues="${memory_issues}Errors: $error_count. "
    fi
    
    # Store results
    if [ -n "$memory_issues" ] || [ $exit_code -ne 0 ]; then
        test_results[$test_name]="FAILED"
        test_issues[$test_name]="$memory_issues Exit code: $exit_code"
        ((failed_tests++))
        log_error "  FAILED: $memory_issues"
    else
        test_results[$test_name]="PASSED"
        test_issues[$test_name]="Clean"
        ((passed_tests++))
        log_info "  PASSED: No memory issues"
    fi
    
    ((total_tests++))
}

generate_summary() {
    log_info "Generating summary report..."
    
    {
        echo "=== Valgrind Test Summary ==="
        echo "Timestamp: $TIMESTAMP"
        echo "Total tests: $total_tests"
        echo "Passed: $passed_tests"
        echo "Failed: $failed_tests"
        echo ""
        
        if [ $failed_tests -gt 0 ]; then
            echo "=== Failed Tests ==="
            for test_name in "${!test_results[@]}"; do
                if [ "${test_results[$test_name]}" == "FAILED" ]; then
                    echo ""
                    echo "Test: $test_name"
                    echo "Issues: ${test_issues[$test_name]}"
                    echo "Log: $LOG_DIR/valgrind_${test_name}_${TIMESTAMP}.log"
                fi
            done | sort
        fi
        
        echo ""
        echo "=== All Test Results ==="
        for test_name in "${!test_results[@]}"; do
            printf "%-40s %s\n" "$test_name:" "${test_results[$test_name]}"
        done | sort
        
    } > "$SUMMARY_FILE"
    
    # Also display the summary
    cat "$SUMMARY_FILE"
}

extract_stack_traces() {
    local test_name="$1"
    local log_file="$LOG_DIR/valgrind_${test_name}_${TIMESTAMP}.log"
    
    if [ -f "$log_file" ]; then
        echo ""
        echo "=== Stack traces for $test_name ==="
        grep -A 10 "at 0x\|by 0x\|Address 0x\|definitely lost\|possibly lost\|indirectly lost" "$log_file" | head -50
    fi
}

create_suppressions_file() {
    local supp_file="valgrind.supp"
    
    if [ ! -f "$supp_file" ]; then
        log_info "Creating valgrind suppressions file: $supp_file"
        
        cat > "$supp_file" << 'EOF'
# Valgrind suppressions for OpenPGP C binding tests
#
# Add suppressions for known false positives here

# FlatCC library initialization
{
   flatcc_init_once
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   ...
   fun:flatcc_*
}

# OpenPGP bridge initialization
{
   openpgp_bridge_init
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   ...
   fun:OpenPGPBridge*
}

# Go runtime false positives
{
   go_runtime_init
   Memcheck:Leak
   match-leak-kinds: reachable
   fun:malloc
   ...
   fun:runtime.*
}
EOF
        
        log_info "Suppressions file created."
    fi
}

# Main execution
main() {
    echo "=== Comprehensive Valgrind Test Runner ==="
    echo "Timestamp: $TIMESTAMP"
    echo ""
    
    check_prerequisites
    create_suppressions_file
    
    # Find all test executables
    test_executables=$(find_test_executables)
    
    echo ""
    log_info "Found $(echo "$test_executables" | wc -l) test executables"
    echo ""
    
    # Run each test under valgrind
    while IFS= read -r test_exe; do
        run_single_test_valgrind "$test_exe"
    done <<< "$test_executables"
    
    echo ""
    generate_summary
    
    # Show detailed stack traces for failed tests
    if [ $failed_tests -gt 0 ]; then
        echo ""
        log_warning "Showing stack traces for failed tests:"
        for test_name in "${!test_results[@]}"; do
            if [ "${test_results[$test_name]}" == "FAILED" ]; then
                extract_stack_traces "$test_name"
            fi
        done
    fi
    
    echo ""
    log_info "Full logs available in: $LOG_DIR"
    log_info "Summary saved to: $SUMMARY_FILE"
    
    # Exit with error if any test failed
    if [ $failed_tests -gt 0 ]; then
        exit 1
    else
        exit 0
    fi
}

# Handle command line arguments
case "$1" in
    --help|-h)
        echo "Usage: $0 [options]"
        echo ""
        echo "Options:"
        echo "  --help, -h       Show this help message"
        echo "  --clean          Clean log directory"
        echo "  --test NAME      Run only specific test"
        echo "  --show-logs      Show log files for failed tests"
        echo ""
        echo "Environment variables:"
        echo "  VALGRIND_OPTS    Additional valgrind options"
        echo "  BUILD_DIR        Directory containing test executables (default: ../build)"
        exit 0
        ;;
    --clean)
        log_info "Cleaning log directory: $LOG_DIR"
        rm -rf "$LOG_DIR"
        mkdir -p "$LOG_DIR"
        log_info "Log directory cleaned"
        exit 0
        ;;
    --test)
        if [ -z "$2" ]; then
            log_error "Test name required"
            exit 1
        fi
        check_prerequisites
        test_exe="$BUILD_DIR/$2"
        if [ ! -f "$test_exe" ]; then
            log_error "Test executable not found: $test_exe"
            exit 1
        fi
        run_single_test_valgrind "$test_exe"
        generate_summary
        exit $?
        ;;
    --show-logs)
        if [ ! -d "$LOG_DIR" ]; then
            log_error "No log directory found"
            exit 1
        fi
        latest_summary=$(ls -t "$LOG_DIR"/valgrind_summary_*.txt 2>/dev/null | head -1)
        if [ -n "$latest_summary" ]; then
            cat "$latest_summary"
        else
            log_error "No summary found"
        fi
        exit 0
        ;;
    "")
        main
        ;;
    *)
        log_error "Unknown option: $1"
        echo "Use --help for usage information"
        exit 1
        ;;
esac