#!/bin/bash

# Simple Valgrind Results Analyzer for Task #2
# Simplified version to complete Task #2 requirements

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"
LOG_DIR="$SCRIPT_DIR/valgrind_logs"
REPORT_FILE="$SCRIPT_DIR/valgrind-report.md"
VALGRIND_RUNNER="$SCRIPT_DIR/valgrind-runner.sh"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    echo "Usage: $0 [--run-tests] [--clean] [--build-dir DIR]"
    echo ""
    echo "Options:"
    echo "  --run-tests    Run valgrind tests before analysis"
    echo "  --clean        Clean previous logs"
    echo "  --build-dir    Custom build directory"
    echo "  --help         Show this help"
}

run_tests_on_all_executables() {
    local build_dir="$1"
    
    log_info "Discovering test executables in: $build_dir"
    
    if [ ! -d "$build_dir" ]; then
        log_error "Build directory not found: $build_dir"
        return 1
    fi
    
    local test_files=$(find "$build_dir" -name "test*" -type f -executable 2>/dev/null | sort)
    
    if [ -z "$test_files" ]; then
        log_error "No test executables found in $build_dir"
        return 1
    fi
    
    log_info "Found $(echo "$test_files" | wc -l) test executable(s)"
    
    # Run valgrind on each test
    while IFS= read -r test_executable; do
        if [ -n "$test_executable" ]; then
            local test_name=$(basename "$test_executable")
            log_info "Running valgrind on: $test_name"
            
            "$VALGRIND_RUNNER" --quiet "$test_executable" || {
                log_error "Issues detected in: $test_name"
            }
        fi
    done <<< "$test_files"
    
    return 0
}

analyze_log_file() {
    local log_file="$1"
    
    if [ ! -f "$log_file" ]; then
        return 1
    fi
    
    local test_name=$(basename "$log_file" | sed 's/valgrind_\(.*\)_[0-9_]*.log/\1/')
    
    # Extract basic error information
    local error_summary=$(grep "ERROR SUMMARY:" "$log_file" | tail -1 || echo "ERROR SUMMARY: 0 errors")
    local error_count=$(echo "$error_summary" | sed 's/.*ERROR SUMMARY: \([0-9]*\).*/\1/' || echo "0")
    
    # Check for memory leaks
    local definitely_lost=$(grep "definitely lost:" "$log_file" | head -1 || echo "")
    local indirectly_lost=$(grep "indirectly lost:" "$log_file" | head -1 || echo "")
    local possibly_lost=$(grep "possibly lost:" "$log_file" | head -1 || echo "")
    
    # Count different error types
    local invalid_reads=$(grep -c "Invalid read" "$log_file" 2>/dev/null || echo "0")
    local invalid_writes=$(grep -c "Invalid write" "$log_file" 2>/dev/null || echo "0")
    local uninit_values=$(grep -c "Conditional jump.*uninitialised\|Use of uninitialised" "$log_file" 2>/dev/null || echo "0")
    
    # Determine priority
    local priority="LOW"
    if [ "$error_count" -gt 0 ] || [ -n "$definitely_lost" ]; then
        priority="HIGH"
    elif [ "$invalid_reads" -gt 0 ] || [ "$invalid_writes" -gt 0 ] || [ -n "$indirectly_lost" ]; then
        priority="MEDIUM"
    fi
    
    # Output results
    echo "$test_name|$error_count|$definitely_lost|$indirectly_lost|$possibly_lost|$invalid_reads|$invalid_writes|$uninit_values|$priority"
    return 0
}generate_report() {
    local timestamp=$(date)
    
    log_info "Generating valgrind analysis report: $REPORT_FILE"
    
    # Create report header
    echo "# Valgrind Analysis Report" > "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    echo "**Generated:** $timestamp" >> "$REPORT_FILE"
    echo "**Log Directory:** $LOG_DIR" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Find log files
    local log_files=$(find "$LOG_DIR" -name "valgrind_*.log" -type f 2>/dev/null | sort)
    
    if [ -z "$log_files" ]; then
        echo "❌ **No valgrind log files found.** Please run valgrind tests first." >> "$REPORT_FILE"
        log_error "No log files found in $LOG_DIR"
        return 1
    fi
    
    # Analyze each log and collect statistics
    local total_tests=0
    local high_priority=0
    local medium_priority=0
    local low_priority=0
    
    echo "## Summary" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    # Create table header
    echo "| Test | Priority | Errors | Memory Leaks | Invalid R/W | Uninit Values |" >> "$REPORT_FILE"
    echo "|------|----------|--------|--------------|-------------|---------------|" >> "$REPORT_FILE"
    
    # Process each log file
    while IFS= read -r log_file; do
        if [ -n "$log_file" ]; then
            local analysis=$(analyze_log_file "$log_file")
            if [ -n "$analysis" ]; then
                local test_name=$(echo "$analysis" | cut -d'|' -f1)
                local error_count=$(echo "$analysis" | cut -d'|' -f2)
                local def_lost=$(echo "$analysis" | cut -d'|' -f3)
                local indir_lost=$(echo "$analysis" | cut -d'|' -f4)
                local poss_lost=$(echo "$analysis" | cut -d'|' -f5)
                local invalid_reads=$(echo "$analysis" | cut -d'|' -f6)
                local invalid_writes=$(echo "$analysis" | cut -d'|' -f7)
                local uninit_values=$(echo "$analysis" | cut -d'|' -f8)
                local priority=$(echo "$analysis" | cut -d'|' -f9)
                
                ((total_tests++))
                
                local priority_icon="✅"
                if [ "$priority" = "HIGH" ]; then
                    priority_icon="🔴"
                    ((high_priority++))
                elif [ "$priority" = "MEDIUM" ]; then
                    priority_icon="🟡"
                    ((medium_priority++))
                else
                    ((low_priority++))
                fi
                
                local leak_summary="None"
                if [ -n "$def_lost" ]; then
                    leak_summary="Definite"
                elif [ -n "$indir_lost" ]; then
                    leak_summary="Indirect"
                elif [ -n "$poss_lost" ]; then
                    leak_summary="Possible"
                fi
                
                local rw_count=$((invalid_reads + invalid_writes))
                
                echo "| $test_name | $priority_icon $priority | $error_count | $leak_summary | $rw_count | $uninit_values |" >> "$REPORT_FILE"
            fi
        fi
    done <<< "$log_files"
    
    # Add statistics
    echo "" >> "$REPORT_FILE"
    echo "**Total Tests:** $total_tests" >> "$REPORT_FILE"
    echo "**High Priority Issues:** $high_priority" >> "$REPORT_FILE"
    echo "**Medium Priority Issues:** $medium_priority" >> "$REPORT_FILE"
    echo "**Clean Tests:** $low_priority" >> "$REPORT_FILE"
    echo "" >> "$REPORT_FILE"
    
    if [ $((high_priority + medium_priority)) -eq 0 ]; then
        echo "✅ **All tests passed!** No memory issues detected." >> "$REPORT_FILE"
    else
        echo "❌ **Memory issues detected** in $((high_priority + medium_priority)) test(s)." >> "$REPORT_FILE"
    fi
    
    return 0
}# Parse arguments
RUN_TESTS=0
CLEAN=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --run-tests)
            RUN_TESTS=1
            shift
            ;;
        --clean)
            CLEAN=1
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Main execution
echo "=== Simple Valgrind Results Analyzer ==="
echo "Build directory: $BUILD_DIR"
echo "Log directory: $LOG_DIR"
echo "Report file: $REPORT_FILE"
echo ""

# Clean logs if requested
if [ "$CLEAN" = "1" ]; then
    log_info "Cleaning log directory: $LOG_DIR"
    rm -rf "$LOG_DIR"
    mkdir -p "$LOG_DIR"
fi

# Run tests if requested
if [ "$RUN_TESTS" = "1" ]; then
    if ! run_tests_on_all_executables "$BUILD_DIR"; then
        log_error "Failed to run valgrind tests"
        exit 1
    fi
    echo ""
fi

# Generate report
if ! generate_report; then
    log_error "Failed to generate report"
    exit 1
fi

log_info "Analysis complete! Report: $REPORT_FILE"

# Check for issues
issues=$(grep -c "🔴\|🟡" "$REPORT_FILE" 2>/dev/null || echo "0")
if [ "$issues" -gt 0 ]; then
    log_error "Memory issues detected! Check report for details."
    exit 1
else
    log_info "✅ No memory issues detected!"
    exit 0
fi