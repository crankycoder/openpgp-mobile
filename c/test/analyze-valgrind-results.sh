#!/bin/bash

# Valgrind Results Analyzer - Automated analysis of valgrind test results
# This script analyzes valgrind logs and generates comprehensive reports

set -e

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/../build"
LOG_DIR="$SCRIPT_DIR/valgrind_logs"
REPORT_FILE="$SCRIPT_DIR/valgrind-report.md"
VALGRIND_RUNNER="$SCRIPT_DIR/valgrind-runner.sh"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

Analyze valgrind test results and generate comprehensive reports.

Options:
  --help, -h         Show this help message
  --build-dir DIR    Directory containing test executables (default: $BUILD_DIR)
  --log-dir DIR      Directory containing valgrind logs (default: $LOG_DIR)
  --output FILE      Output report file (default: $REPORT_FILE)
  --run-tests        Run valgrind tests before analysis
  --clean            Clean previous logs before running

Examples:
  $0                              # Analyze existing logs
  $0 --run-tests                 # Run tests and analyze
  $0 --clean --run-tests         # Clean, run, and analyze
  $0 --output custom-report.md   # Custom output file

Exit codes:
  0   Analysis completed successfully
  1   Analysis failed or memory issues found
EOF
}

discover_test_executables() {
    local build_dir="$1"
    
    if [ ! -d "$build_dir" ]; then
        log_error "Build directory not found: $build_dir"
        return 1
    fi
    
    local test_files=$(find "$build_dir" -name "test*" -type f -executable 2>/dev/null | sort)
    
    if [ -z "$test_files" ]; then
        log_warning "No test executables found in $build_dir"
        return 1
    fi
    
    echo "$test_files"
    return 0
}

run_valgrind_tests() {
    local build_dir="$1"
    
    log_section "Running valgrind tests on all executables"
    
    local test_executables
    if ! test_executables=$(discover_test_executables "$build_dir"); then
        return 1
    fi
    
    log_info "Found $(echo "$test_executables" | wc -l) test executable(s)"
    
    local test_count=0
    while IFS= read -r test_executable; do
        if [ -n "$test_executable" ]; then
            local test_name=$(basename "$test_executable")
            log_info "Running valgrind on: $test_name"
            
            ((test_count++))
            
            if ! "$VALGRIND_RUNNER" --quiet "$test_executable"; then
                log_warning "Issues detected in: $test_name"
            fi
        fi
    done <<< "$test_executables"
    
    log_info "Completed valgrind runs on $test_count test(s)"
    return 0
}

analyze_single_log() {
    local log_file="$1"
    local test_name=$(basename "$log_file" | sed 's/valgrind_\(.*\)_[0-9_]*.log/\1/')
    
    if [ ! -f "$log_file" ]; then
        return 1
    fi
    
    # Extract error counts
    local error_summary=$(grep "ERROR SUMMARY:" "$log_file" | tail -1)
    local error_count=$(echo "$error_summary" | sed 's/.*ERROR SUMMARY: \([0-9]*\).*/\1/' 2>/dev/null || echo "0")
    
    # Extract leak information
    local definitely_lost=$(grep "definitely lost:" "$log_file" | sed 's/.*definitely lost: \([0-9,]*\) bytes.*/\1/' 2>/dev/null || echo "0")
    local indirectly_lost=$(grep "indirectly lost:" "$log_file" | sed 's/.*indirectly lost: \([0-9,]*\) bytes.*/\1/' 2>/dev/null || echo "0")
    local possibly_lost=$(grep "possibly lost:" "$log_file" | sed 's/.*possibly lost: \([0-9,]*\) bytes.*/\1/' 2>/dev/null || echo "0")
    
    # Count invalid reads/writes
    local invalid_reads=$(grep -c "Invalid read" "$log_file" 2>/dev/null || echo "0")
    local invalid_writes=$(grep -c "Invalid write" "$log_file" 2>/dev/null || echo "0")
    
    # Count uninitialized values
    local uninit_values=$(grep -c "Conditional jump.*uninitialised\|Use of uninitialised" "$log_file" 2>/dev/null || echo "0")
    
    # Extract stack traces for errors
    local error_functions=""
    if [ "${error_count:-0}" -gt 0 ]; then
        error_functions=$(grep -A 10 "Invalid read\|Invalid write\|definitely lost\|indirectly lost" "$log_file" | \
                         grep "fun:" | sed 's/.*fun:\([^)]*\).*/\1/' | sort | uniq | head -5 | tr '\n' ',' | sed 's/,$//')
    fi
    
    # Determine severity
    local severity="LOW"
    if [ "${error_count:-0}" -gt 0 ] || [ "${definitely_lost:-0}" != "0" ]; then
        severity="HIGH"
    elif [ "${indirectly_lost:-0}" != "0" ] || [ "${invalid_reads:-0}" -gt 0 ] || [ "${invalid_writes:-0}" -gt 0 ]; then
        severity="MEDIUM"
    fi
    
    # Output structured data
    echo "$test_name|$error_count|$definitely_lost|$indirectly_lost|$possibly_lost|$invalid_reads|$invalid_writes|$uninit_values|$severity|$error_functions"
}generate_comprehensive_report() {
    local timestamp=$(date)
    
    log_section "Generating comprehensive valgrind analysis report"
    
    # Create report header
    cat > "$REPORT_FILE" << EOF
# Comprehensive Valgrind Analysis Report

**Generated:** $timestamp  
**Analysis Tool:** analyze-valgrind-results.sh  
**Log Directory:** $LOG_DIR  

## Executive Summary

EOF

    # Find all log files
    local log_files=$(find "$LOG_DIR" -name "valgrind_*.log" -type f 2>/dev/null | sort)
    local total_tests=0
    local tests_with_issues=0
    local high_severity=0
    local medium_severity=0
    
    if [ -z "$log_files" ]; then
        echo "❌ **No valgrind log files found.** Please run valgrind tests first." >> "$REPORT_FILE"
        return 1
    fi
    
    # Analyze each log file and collect data
    local analysis_data=""
    while IFS= read -r log_file; do
        if [ -n "$log_file" ]; then
            local analysis=$(analyze_single_log "$log_file")
            if [ -n "$analysis" ]; then
                analysis_data="$analysis_data$analysis\n"
                ((total_tests++))
                
                local severity=$(echo "$analysis" | cut -d'|' -f9)
                if [ "$severity" = "HIGH" ]; then
                    ((tests_with_issues++))
                    ((high_severity++))
                elif [ "$severity" = "MEDIUM" ]; then
                    ((tests_with_issues++))
                    ((medium_severity++))
                fi
            fi
        fi
    done <<< "$log_files"
    
    # Generate summary
    cat >> "$REPORT_FILE" << EOF
**Total Tests Analyzed:** $total_tests  
**Tests with Memory Issues:** $tests_with_issues  
**High Severity Issues:** $high_severity  
**Medium Severity Issues:** $medium_severity  

EOF

    if [ "$tests_with_issues" -eq 0 ]; then
        echo "✅ **All tests passed!** No memory issues detected." >> "$REPORT_FILE"
    else
        echo "❌ **Memory issues detected** in $tests_with_issues test(s)." >> "$REPORT_FILE"
    fi
    
    # Generate detailed analysis table
    cat >> "$REPORT_FILE" << EOF

## Detailed Test Analysis

| Test | Severity | Errors | Definitely Lost | Invalid R/W | Uninit Values | Problem Functions |
|------|----------|--------|-----------------|-------------|---------------|-------------------|
EOF

    # Process analysis data and generate table rows
    if [ -n "$analysis_data" ]; then
        echo -e "$analysis_data" | while IFS='|' read -r test_name error_count def_lost indir_lost poss_lost invalid_reads invalid_writes uninit_values severity functions; do
            if [ -n "$test_name" ]; then
                local rw_count=$((invalid_reads + invalid_writes))
                local status_icon="✅"
                if [ "$severity" = "HIGH" ]; then
                    status_icon="🔴"
                elif [ "$severity" = "MEDIUM" ]; then
                    status_icon="🟡"
                fi
                
                echo "| $test_name | $status_icon $severity | $error_count | $def_lost bytes | $rw_count | $uninit_values | ${functions:-None} |" >> "$REPORT_FILE"
            fi
        done
    fi
    
    return 0
}generate_detailed_analysis() {
    cat >> "$REPORT_FILE" << EOF

## Priority Issues by Severity

### 🔴 High Severity Issues

EOF

    # Find high severity issues
    find "$LOG_DIR" -name "valgrind_*.log" -type f 2>/dev/null | while read -r log_file; do
        if [ -n "$log_file" ]; then
            local analysis=$(analyze_single_log "$log_file")
            local severity=$(echo "$analysis" | cut -d'|' -f9)
            
            if [ "$severity" = "HIGH" ]; then
                local test_name=$(echo "$analysis" | cut -d'|' -f1)
                local error_count=$(echo "$analysis" | cut -d'|' -f2)
                local def_lost=$(echo "$analysis" | cut -d'|' -f3)
                
                cat >> "$REPORT_FILE" << EOF

#### $test_name

**Issues:** $error_count errors, $def_lost bytes definitely lost  
**Log file:** $log_file  

**Error summary:**
\`\`\`
$(grep "ERROR SUMMARY:" "$log_file" | tail -1)
\`\`\`

**Memory leaks:**
\`\`\`
$(grep -A 3 -B 1 "definitely lost" "$log_file" | head -10)
\`\`\`

**Stack traces:**
\`\`\`
$(grep -A 5 "Invalid read\|Invalid write\|definitely lost" "$log_file" | head -15)
\`\`\`

EOF
            fi
        fi
    done
    
    cat >> "$REPORT_FILE" << EOF

### 🟡 Medium Severity Issues

EOF

    # Find medium severity issues
    find "$LOG_DIR" -name "valgrind_*.log" -type f 2>/dev/null | while read -r log_file; do
        if [ -n "$log_file" ]; then
            local analysis=$(analyze_single_log "$log_file")
            local severity=$(echo "$analysis" | cut -d'|' -f9)
            
            if [ "$severity" = "MEDIUM" ]; then
                local test_name=$(echo "$analysis" | cut -d'|' -f1)
                local invalid_reads=$(echo "$analysis" | cut -d'|' -f6)
                local invalid_writes=$(echo "$analysis" | cut -d'|' -f7)
                
                cat >> "$REPORT_FILE" << EOF

#### $test_name

**Issues:** $invalid_reads invalid reads, $invalid_writes invalid writes  
**Log file:** $log_file  

**Sample errors:**
\`\`\`
$(grep -A 2 "Invalid read\|Invalid write" "$log_file" | head -8)
\`\`\`

EOF
            fi
        fi
    done
}

# Parse command line arguments
RUN_TESTS=0
CLEAN=0

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
        --log-dir)
            LOG_DIR="$2"
            shift 2
            ;;
        --output)
            REPORT_FILE="$2"
            shift 2
            ;;
        --run-tests)
            RUN_TESTS=1
            shift
            ;;
        --clean)
            CLEAN=1
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
done

# Main execution
main() {
    echo "=== Valgrind Results Analyzer ==="
    echo "Build directory: $BUILD_DIR"
    echo "Log directory: $LOG_DIR"
    echo "Report file: $REPORT_FILE"
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
    
    # Run valgrind tests if requested
    if [ "$RUN_TESTS" = "1" ]; then
        run_valgrind_tests "$BUILD_DIR" || {
            log_error "Failed to run valgrind tests"
            exit 1
        }
        echo ""
    fi
    
    # Generate comprehensive report
    generate_comprehensive_report || {
        log_error "Failed to generate report"
        exit 1
    }
    
    # Generate detailed analysis
    generate_detailed_analysis
    
    echo ""
    log_info "Analysis complete!"
    log_info "Report generated: $REPORT_FILE"
    
    # Check if issues were found
    local issues_found=$(grep -c "🔴\|🟡" "$REPORT_FILE" 2>/dev/null || echo "0")
    if [ "$issues_found" -gt 0 ]; then
        log_warning "Memory issues detected! Review the report for details."
        exit 1
    else
        log_info "✅ No memory issues detected!"
        exit 0
    fi
}

# Run main function
main "$@"