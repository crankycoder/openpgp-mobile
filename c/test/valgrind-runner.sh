#!/bin/bash

# Valgrind Runner - Generic valgrind test runner for any test executable
# This script runs any test executable under valgrind with proper error detection

set -e

# Configuration
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"
VALGRIND_SUPPRESSIONS=""
LOG_DIR="./valgrind_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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

show_help() {
    cat << EOF
Usage: $0 [options] <test_executable>

Run a test executable under valgrind with memory error detection.

Arguments:
  test_executable    Path to the test executable to run

Options:
  --help, -h         Show this help message
  --clean            Clean log directory before running
  --suppressions FILE Use custom suppressions file
  --output-dir DIR   Use custom output directory for logs
  --quiet           Suppress verbose output

Environment variables:
  VALGRIND_OPTS     Additional valgrind options (default: $VALGRIND_OPTS)

Examples:
  $0 ../build/test_generate
  $0 --suppressions custom.supp ../build/test_encrypt
  $0 --clean --output-dir /tmp/valgrind ../build/test_symmetric

Exit codes:
  0   No memory errors detected
  1   Memory errors detected or other failure
EOF
}

check_prerequisites() {
    log_info "Checking prerequisites..."
    
    # Check if valgrind is available
    if ! command -v valgrind &> /dev/null; then
        log_error "Valgrind is not installed or not in PATH"
        log_info "Install valgrind: sudo apt-get install valgrind"
        exit 1
    fi
    
    log_info "Prerequisites check passed"
}

setup_environment() {
    # Create log directory
    mkdir -p "$LOG_DIR"
    
    # Check if suppressions file exists
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local default_supp="$script_dir/valgrind.supp"
    
    if [ -z "$VALGRIND_SUPPRESSIONS" ] && [ -f "$default_supp" ]; then
        VALGRIND_SUPPRESSIONS="--suppressions=$default_supp"
        log_info "Using default suppressions file: $default_supp"
    elif [ -n "$VALGRIND_SUPPRESSIONS" ]; then
        log_info "Using custom suppressions: $VALGRIND_SUPPRESSIONS"
    fi
}

run_valgrind_test() {
    local test_executable="$1"
    local test_name=$(basename "$test_executable")
    local log_file="$LOG_DIR/valgrind_${test_name}_${TIMESTAMP}.log"
    
    # Check if test executable exists
    if [ ! -f "$test_executable" ]; then
        log_error "Test executable not found: $test_executable"
        return 1
    fi
    
    # Check if test executable is executable
    if [ ! -x "$test_executable" ]; then
        log_error "Test executable is not executable: $test_executable"
        return 1
    fi
    
    log_info "Running valgrind test: $test_name"
    log_info "Test executable: $test_executable"
    log_info "Log file: $log_file"
    
    # Run valgrind
    local cmd="valgrind $VALGRIND_OPTS $VALGRIND_SUPPRESSIONS $test_executable"
    
    echo "Command: $cmd" > "$log_file"
    echo "Timestamp: $(date)" >> "$log_file"
    echo "========================" >> "$log_file"
    
    if [ "$QUIET" = "1" ]; then
        if $cmd >> "$log_file" 2>&1; then
            log_info "Valgrind test PASSED: $test_name"
            return 0
        else
            local exit_code=$?
            log_error "Valgrind test FAILED: $test_name (exit code: $exit_code)"
            return $exit_code
        fi
    else
        log_info "Running: $cmd"
        if $cmd 2>&1 | tee -a "$log_file"; then
            log_info "Valgrind test PASSED: $test_name"
            return 0
        else
            local exit_code=$?
            log_error "Valgrind test FAILED: $test_name (exit code: $exit_code)"
            return $exit_code
        fi
    fi
}

analyze_results() {
    local log_file="$1"
    
    if [ ! -f "$log_file" ]; then
        log_warning "Log file not found: $log_file"
        return 1
    fi
    
    log_info "Analyzing results..."
    
    # Check for errors
    local error_count=$(grep "ERROR SUMMARY:" "$log_file" | tail -1 | sed 's/.*ERROR SUMMARY: \([0-9]*\).*/\1/')
    local leak_count=$(grep -c "definitely lost\|indirectly lost\|possibly lost" "$log_file" || true)
    
    echo ""
    log_info "=== Valgrind Analysis ==="
    echo "Errors detected: ${error_count:-0}"
    echo "Memory leaks: ${leak_count:-0}"
    
    if [ "${error_count:-0}" -gt 0 ] || [ "${leak_count:-0}" -gt 0 ]; then
        log_error "Memory issues detected!"
        echo ""
        log_warning "Check the full log for details: $log_file"
        return 1
    else
        log_info "No memory issues detected!"
        return 0
    fi
}

# Parse command line arguments
QUIET=0
CLEAN=0
TEST_EXECUTABLE=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --suppressions)
            VALGRIND_SUPPRESSIONS="--suppressions=$2"
            shift 2
            ;;
        --output-dir)
            LOG_DIR="$2"
            shift 2
            ;;
        --quiet)
            QUIET=1
            shift
            ;;
        -*)
            log_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
        *)
            if [ -z "$TEST_EXECUTABLE" ]; then
                TEST_EXECUTABLE="$1"
            else
                log_error "Multiple test executables specified"
                echo "Use --help for usage information"
                exit 1
            fi
            shift
            ;;
    esac
done

# Validate arguments
if [ -z "$TEST_EXECUTABLE" ]; then
    log_error "No test executable specified"
    echo "Use --help for usage information"
    exit 1
fi

# Main execution
main() {
    echo "=== Valgrind Runner ==="
    echo "Test executable: $TEST_EXECUTABLE"
    echo "Timestamp: $TIMESTAMP"
    echo ""
    
    check_prerequisites
    setup_environment
    
    # Clean logs if requested
    if [ "$CLEAN" = "1" ]; then
        log_info "Cleaning log directory: $LOG_DIR"
        rm -rf "$LOG_DIR"
        mkdir -p "$LOG_DIR"
    fi
    
    # Run the valgrind test
    local log_file="$LOG_DIR/valgrind_$(basename "$TEST_EXECUTABLE")_${TIMESTAMP}.log"
    local exit_code=0
    
    run_valgrind_test "$TEST_EXECUTABLE" || exit_code=$?
    
    # Analyze results
    analyze_results "$log_file" || exit_code=$?
    
    echo ""
    if [ $exit_code -eq 0 ]; then
        log_info "Valgrind test completed successfully!"
    else
        log_error "Valgrind test failed!"
        log_info "Check log file: $log_file"
    fi
    
    exit $exit_code
}

# Run main function
main "$@"