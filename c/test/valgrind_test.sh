#!/bin/bash

# Valgrind Test Runner for FlatBuffer Memory Tests
# This script runs the FlatBuffer memory tests under valgrind
# and captures the output for analysis

set -e

# Configuration
VALGRIND_OPTS="--leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1"
VALGRIND_SUPPRESSIONS=""
TEST_EXECUTABLE="./test_flatbuffer_memory"
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

check_prerequisites() {
    log_info "Checking prerequisites..."
    
    # Check if valgrind is available
    if ! command -v valgrind &> /dev/null; then
        log_error "Valgrind is not installed or not in PATH"
        exit 1
    fi
    
    # Check if test executable exists
    if [ ! -f "$TEST_EXECUTABLE" ]; then
        log_error "Test executable not found: $TEST_EXECUTABLE"
        log_info "Please build the test executable first with: make test-flatbuffer-memory"
        exit 1
    fi
    
    # Create log directory
    mkdir -p "$LOG_DIR"
    
    log_info "Prerequisites check passed"
}

run_valgrind_test() {
    local test_name="$1"
    local log_file="$LOG_DIR/valgrind_${test_name}_${TIMESTAMP}.log"
    
    log_info "Running valgrind test: $test_name"
    log_info "Log file: $log_file"
    
    # Check if suppressions file exists
    if [ -f "valgrind.supp" ]; then
        VALGRIND_SUPPRESSIONS="--suppressions=valgrind.supp"
        log_info "Using suppressions file: valgrind.supp"
    fi
    
    # Run valgrind
    local cmd="valgrind $VALGRIND_OPTS $VALGRIND_SUPPRESSIONS $TEST_EXECUTABLE"
    
    echo "Running: $cmd" > "$log_file"
    echo "========================" >> "$log_file"
    
    if $cmd >> "$log_file" 2>&1; then
        log_info "Valgrind test PASSED: $test_name"
        return 0
    else
        local exit_code=$?
        log_error "Valgrind test FAILED: $test_name (exit code: $exit_code)"
        
        # Show relevant parts of the log
        echo ""
        log_warning "Valgrind output (last 50 lines):"
        tail -n 50 "$log_file"
        echo ""
        
        return $exit_code
    fi
}

analyze_valgrind_logs() {
    log_info "Analyzing valgrind logs..."
    
    local total_logs=$(find "$LOG_DIR" -name "valgrind_*_${TIMESTAMP}.log" | wc -l)
    local error_logs=$(find "$LOG_DIR" -name "valgrind_*_${TIMESTAMP}.log" -exec grep -l "ERROR SUMMARY: [^0]" {} \; | wc -l)
    local leak_logs=$(find "$LOG_DIR" -name "valgrind_*_${TIMESTAMP}.log" -exec grep -l "definitely lost\|indirectly lost\|possibly lost" {} \; | wc -l)
    
    echo ""
    log_info "=== Valgrind Analysis Summary ==="
    echo "Total tests run: $total_logs"
    echo "Tests with errors: $error_logs"
    echo "Tests with memory leaks: $leak_logs"
    
    if [ "$error_logs" -gt 0 ] || [ "$leak_logs" -gt 0 ]; then
        log_error "Memory issues detected!"
        echo ""
        log_info "Files with issues:"
        find "$LOG_DIR" -name "valgrind_*_${TIMESTAMP}.log" -exec grep -l "ERROR SUMMARY: [^0]\|definitely lost\|indirectly lost\|possibly lost" {} \;
        return 1
    else
        log_info "No memory issues detected!"
        return 0
    fi
}

create_suppressions_template() {
    local supp_file="valgrind.supp"
    
    if [ ! -f "$supp_file" ]; then
        log_info "Creating valgrind suppressions template: $supp_file"
        
        cat > "$supp_file" << 'EOF'
# Valgrind suppressions for FlatBuffer memory tests
#
# Add suppressions for known false positives here
# Format:
# {
#    suppression_name
#    Memcheck:Leak
#    match-leak-kinds: definite
#    fun:malloc
#    fun:some_function
# }

# Example: Suppress known library leaks
# {
#    flatcc_library_init
#    Memcheck:Leak
#    match-leak-kinds: definite
#    fun:malloc
#    fun:flatcc_*
# }
EOF
        
        log_info "Suppressions template created. Edit $supp_file to add specific suppressions."
    fi
}

# Main execution
main() {
    echo "=== Valgrind Test Runner for FlatBuffer Memory Tests ==="
    echo "Timestamp: $TIMESTAMP"
    echo ""
    
    check_prerequisites
    create_suppressions_template
    
    # Run the valgrind test
    local exit_code=0
    run_valgrind_test "flatbuffer_memory" || exit_code=$?
    
    # Analyze results
    analyze_valgrind_logs || exit_code=$?
    
    echo ""
    if [ $exit_code -eq 0 ]; then
        log_info "All valgrind tests PASSED!"
    else
        log_error "Some valgrind tests FAILED!"
        log_info "Check log files in $LOG_DIR for details"
    fi
    
    exit $exit_code
}

# Handle command line arguments
case "$1" in
    --help|-h)
        echo "Usage: $0 [options]"
        echo ""
        echo "Options:"
        echo "  --help, -h     Show this help message"
        echo "  --clean        Clean log directory"
        echo ""
        echo "Environment variables:"
        echo "  VALGRIND_OPTS  Additional valgrind options"
        echo "  TEST_EXECUTABLE Path to test executable (default: ./test_flatbuffer_memory)"
        exit 0
        ;;
    --clean)
        log_info "Cleaning log directory: $LOG_DIR"
        rm -rf "$LOG_DIR"
        mkdir -p "$LOG_DIR"
        log_info "Log directory cleaned"
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