# OpenPGP C Wrapper Library Makefile

CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99 -fPIC -g
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

# Static library
$(STATIC_LIB): $(OBJECTS)
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
$(TEST_RUNNER): $(TEST_OBJECTS) $(STATIC_LIB)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ -ldl

# Run tests
test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR) $(LIBDIR)

# Clean everything including generated files
distclean: clean
	rm -rf $(GENERATED_DIR)

# Install (placeholder for now)
install: all
	@echo "Install target not implemented yet"

# Generate FlatBuffers headers (will be implemented in Task 0.2)
generate:
	@echo "FlatBuffers generation not implemented yet"

.PHONY: all directories test clean distclean install generate