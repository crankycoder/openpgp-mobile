# Memory Regression Test Suite Makefile
# Phase 7.7: Comprehensive Valgrind Error Detection and Fixes

# Memory test executables
MEMORY_TESTS = test_memory_error_paths test_memory_large_data test_memory_stress \
               test_memory_edge_cases test_memory_performance test_memory_regression_suite

# Common dependencies for memory tests
MEMORY_TEST_DEPS = $(TESTDIR)/test_isolation_manager.c $(TESTDIR)/memory_helpers.c \
                   $(SRCDIR)/openpgp.c $(LIBDIR)/libflatccrt.a

# Common compilation flags for memory tests
MEMORY_TEST_CFLAGS = $(CFLAGS) -I$(INCDIR) -I$(GENERATED_DIR) -I../output/binding
MEMORY_TEST_LDFLAGS = -ldl

# Memory test executables with build path
MEMORY_TEST_EXECUTABLES = $(addprefix $(BUILDDIR)/,$(MEMORY_TESTS))

# Build individual memory test executables
$(BUILDDIR)/test_memory_error_paths: $(TESTDIR)/test_memory_error_paths.c $(MEMORY_TEST_DEPS) | directories
	$(CC) $(MEMORY_TEST_CFLAGS) -o $@ $< $(MEMORY_TEST_DEPS) $(MEMORY_TEST_LDFLAGS)

$(BUILDDIR)/test_memory_large_data: $(TESTDIR)/test_memory_large_data.c $(MEMORY_TEST_DEPS) | directories  
	$(CC) $(MEMORY_TEST_CFLAGS) -o $@ $< $(MEMORY_TEST_DEPS) $(MEMORY_TEST_LDFLAGS)

$(BUILDDIR)/test_memory_stress: $(TESTDIR)/test_memory_stress.c $(MEMORY_TEST_DEPS) | directories
	$(CC) $(MEMORY_TEST_CFLAGS) -o $@ $< $(MEMORY_TEST_DEPS) $(MEMORY_TEST_LDFLAGS)

$(BUILDDIR)/test_memory_edge_cases: $(TESTDIR)/test_memory_edge_cases.c $(MEMORY_TEST_DEPS) | directories
	$(CC) $(MEMORY_TEST_CFLAGS) -o $@ $< $(MEMORY_TEST_DEPS) $(MEMORY_TEST_LDFLAGS)

$(BUILDDIR)/test_memory_performance: $(TESTDIR)/test_memory_performance.c $(MEMORY_TEST_DEPS) | directories
	$(CC) $(MEMORY_TEST_CFLAGS) -o $@ $< $(MEMORY_TEST_DEPS) $(MEMORY_TEST_LDFLAGS)

$(BUILDDIR)/test_memory_regression_suite: $(TESTDIR)/test_memory_regression_suite.c $(MEMORY_TEST_DEPS) | directories
	$(CC) $(MEMORY_TEST_CFLAGS) -o $@ $< $(MEMORY_TEST_DEPS) $(MEMORY_TEST_LDFLAGS)

# Build all memory tests
memory-tests: $(MEMORY_TEST_EXECUTABLES)
	@echo "Built all memory regression tests"# Run all memory tests
memory-test-run: $(MEMORY_TEST_EXECUTABLES)
	@echo "Running Memory Regression Test Suite..."
	@echo "========================================"
	@for test in $(MEMORY_TESTS); do \
		echo ""; \
		echo "Running $$test..."; \
		echo "-------------------"; \
		$(BUILDDIR)/$$test || echo "$$test FAILED"; \
	done
	@echo ""
	@echo "Memory regression test suite completed"

# Run comprehensive memory regression suite
memory-test-suite: $(BUILDDIR)/test_memory_regression_suite
	@echo "Running comprehensive memory regression suite..."
	$(BUILDDIR)/test_memory_regression_suite

# Run memory tests under valgrind
memory-test-valgrind: $(MEMORY_TEST_EXECUTABLES)
	@echo "Running Memory Regression Tests under Valgrind..."
	@echo "=================================================="
	@for test in $(MEMORY_TESTS); do \
		echo ""; \
		echo "Running $$test under valgrind..."; \
		echo "--------------------------------"; \
		valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $(BUILDDIR)/$$test || echo "$$test FAILED valgrind check"; \
	done

# Run individual memory test under valgrind
memory-test-valgrind-single:
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make memory-test-valgrind-single TEST=test_memory_xxx"; \
		exit 1; \
	fi
	@echo "Running $(TEST) under valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $(BUILDDIR)/$(TEST)

# Clean memory test executables
memory-tests-clean:
	@echo "Cleaning memory test executables..."
	rm -f $(MEMORY_TEST_EXECUTABLES)

# Help for memory test targets
memory-test-help:
	@echo "Memory Regression Test Suite Targets:"
	@echo "  memory-tests                 - Build all memory regression tests"
	@echo "  memory-test-run             - Run all memory tests individually"
	@echo "  memory-test-suite           - Run comprehensive regression suite"
	@echo "  memory-test-valgrind        - Run all memory tests under valgrind"
	@echo "  memory-test-valgrind-single - Run single test under valgrind (TEST=name)"
	@echo "  memory-tests-clean          - Clean memory test executables"

.PHONY: memory-tests memory-test-run memory-test-suite memory-test-valgrind memory-test-valgrind-single memory-tests-clean memory-test-help