# OpenPGP C Wrapper Library Makefile

CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99 -fPIC -g -Wno-misleading-indentation
LDFLAGS ?= -shared
AR ?= ar
RANLIB ?= ranlib

# Directories
SRCDIR = src
INCDIR = include
TESTDIR = test
EXAMPLESDIR = examples
BUILDDIR = build
LIBDIR = lib
GENERATED_DIR = generated

# Library names
LIBNAME = libopenpgp
STATIC_LIB = $(LIBDIR)/$(LIBNAME).a
SHARED_LIB = $(LIBDIR)/$(LIBNAME).so

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

# Test files
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(BUILDDIR)/test_%.o)
TEST_RUNNER = $(BUILDDIR)/test_runner

# Include paths
INCLUDES = -I$(INCDIR) -I$(GENERATED_DIR) -I../output/binding

# Default target
all: directories $(STATIC_LIB) $(SHARED_LIB)

# Create necessary directories
directories:
	@mkdir -p $(BUILDDIR) $(LIBDIR) $(GENERATED_DIR)

# Static library (exclude flatcc_stubs.o)
$(STATIC_LIB): $(filter-out build/flatcc_stubs.o, $(OBJECTS))
	$(AR) rcs $@ $^
	$(RANLIB) $@

# Shared library
$(SHARED_LIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile test files
$(BUILDDIR)/test_%.o: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test runner
$(TEST_RUNNER): $(TEST_OBJECTS) $(STATIC_LIB) $(LIBDIR)/libflatccrt.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ -ldl

# Run tests
test: $(TEST_RUNNER)
	LD_LIBRARY_PATH=../output/binding:$$LD_LIBRARY_PATH ./$(TEST_RUNNER)

# Debug test executables
DEBUG_TEST_SOURCES = $(TESTDIR)/test_sign_verify_debug.c $(TESTDIR)/test_sign_verify_minimal.c
DEBUG_TEST_EXECUTABLES = $(DEBUG_TEST_SOURCES:$(TESTDIR)/%.c=$(BUILDDIR)/%)

# Compile debug tests
$(BUILDDIR)/test_sign_verify_debug: $(TESTDIR)/test_sign_verify_debug.c $(STATIC_LIB) $(LIBDIR)/libflatccrt.a
	$(CC) $(CFLAGS) $(INCLUDES) -DDEBUG_VERBOSE=1 -DDEBUG_MEMORY=1 -o $@ $^ -ldl

$(BUILDDIR)/test_sign_verify_minimal: $(TESTDIR)/test_sign_verify_minimal.c $(STATIC_LIB) $(LIBDIR)/libflatccrt.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ -ldl

# Run debug tests
test-debug: $(BUILDDIR)/test_sign_verify_debug
	@echo "Running debug test harness..."
	cd .. && LD_LIBRARY_PATH=output/binding:$$LD_LIBRARY_PATH ./c/$(BUILDDIR)/test_sign_verify_debug

test-minimal: $(BUILDDIR)/test_sign_verify_minimal
	@echo "Running minimal debug tests..."
	cd .. && LD_LIBRARY_PATH=output/binding:$$LD_LIBRARY_PATH ./c/$(BUILDDIR)/test_sign_verify_minimal

# Run both debug tests
test-all-debug: test-debug test-minimal

# Memory test executable
$(BUILDDIR)/test_flatbuffer_memory: $(TESTDIR)/test_flatbuffer_memory.c $(TESTDIR)/memory_helpers.c $(STATIC_LIB) $(LIBDIR)/libflatccrt.a
	$(CC) $(CFLAGS) $(INCLUDES) -DDEBUG_MEMORY=1 -o $@ $^ -ldl

# Serialize request test executable
$(BUILDDIR)/test_serialize_request: $(TESTDIR)/test_serialize_request.c $(TESTDIR)/memory_helpers.c $(STATIC_LIB) $(LIBDIR)/libflatccrt.a
	$(CC) $(CFLAGS) $(INCLUDES) -DDEBUG_MEMORY=1 -o $@ $^ -ldl

# Run memory tests
test-memory: $(BUILDDIR)/test_flatbuffer_memory
	@echo "Running FlatBuffer memory tests..."
	cd .. && LD_LIBRARY_PATH=output/binding:$$LD_LIBRARY_PATH ./c/$(BUILDDIR)/test_flatbuffer_memory

# Run serialize request tests
test-serialize: $(BUILDDIR)/test_serialize_request
	@echo "Running serialize_generate_request tests..."
	cd .. && LD_LIBRARY_PATH=output/binding:$$LD_LIBRARY_PATH ./c/$(BUILDDIR)/test_serialize_request

# Run memory tests with valgrind
test-memory-valgrind: $(BUILDDIR)/test_flatbuffer_memory
	@echo "Running FlatBuffer memory tests with valgrind..."
	cd $(TESTDIR) && ./valgrind_test.sh

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(LIBDIR)

# Clean everything including generated files
distclean: clean
	rm -rf $(GENERATED_DIR)

# Install (placeholder for now)
install: all
	@echo "Install target not implemented yet"

# Generate FlatBuffers headers
generate:
	$(MAKE) -C .. flatbuffers_c_wrapper

.PHONY: all directories test clean distclean install generate test-memory test-memory-valgrind test-debug test-minimal test-all-debug