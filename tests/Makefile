# ************************************************************************** #
#                                                                            #
#                                                        :::      ::::::::   #
#   Makefile                                           :+:      :+:    :+:   #
#                                                    +:+ +:+         +:+     #
#   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        #
#                                                +#+#+#+#+#+   +#+           #
#   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             #
#   Updated: 2025/09/18 11:17:46 by rluiz            ###   ########.fr       #
#                                                                            #
# ************************************************************************** #

# ================================ COMPILER ================================= #
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g3
LDFLAGS = 

# ================================ DIRECTORIES ============================== #
# Project structure following NASA architecture
SRCDIR = .
INCDIR = ../include
LIBDIR = ../lib
BUILDDIR = ../build
OBJDIR = $(BUILDDIR)/test_obj
BINDIR = $(BUILDDIR)/test_bin

# Library paths
LIBFT_DIR = $(LIBDIR)
MALLOC_LIB_DIR = $(BUILDDIR)/bin

# Include paths
INCLUDES = -I$(INCDIR) -I$(LIBDIR) -I$(SRCDIR)

# Library linking
LIBPATH = -L$(MALLOC_LIB_DIR) -L$(LIBFT_DIR)
LIBS = -lft_malloc -lft

# ================================ SOURCE FILES ============================= #
# Core test modules following NASA modular design
CORE_TEST_SRCS = test_malloc.c test_free.c test_realloc.c
INTEGRATION_SRCS = test_runner.c test_all.c
SUPPORT_SRCS = test_common.h

# Object files
CORE_TEST_OBJS = $(CORE_TEST_SRCS:%.c=$(OBJDIR)/%.o)
INTEGRATION_OBJS = $(INTEGRATION_SRCS:%.c=$(OBJDIR)/%.o)

# ================================ EXECUTABLES =============================== #
# Individual test executables (NASA principle: testable components)
MALLOC_TEST_BIN = $(BINDIR)/test_malloc
FREE_TEST_BIN = $(BINDIR)/test_free
REALLOC_TEST_BIN = $(BINDIR)/test_realloc

# Coordinated test executables
RUNNER_BIN = $(BINDIR)/test_runner
ALL_TEST_BIN = $(BINDIR)/test_all

# All executables
ALL_BINS = $(MALLOC_TEST_BIN) $(FREE_TEST_BIN) $(REALLOC_TEST_BIN) $(RUNNER_BIN) $(ALL_TEST_BIN)

# ================================ BUILD RULES ============================== #
.PHONY: all clean fclean re test test-quick test-perf test-stress test-leaks test-valgrind
.PHONY: test-malloc test-free test-realloc test-runner test-all help

# Default target - build all test executables
all: $(ALL_BINS)

# Create necessary directories
$(OBJDIR) $(BINDIR):
	@mkdir -p $@

# ================================ INDIVIDUAL TESTS ========================= #
# NASA principle: Each component should be independently testable

$(MALLOC_TEST_BIN): test_malloc.c | $(BINDIR)
	@echo "🔨 Building malloc test executable..."
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_MALLOC_STANDALONE -o $@ $< $(LIBPATH) $(LIBS)

$(FREE_TEST_BIN): test_free.c | $(BINDIR)
	@echo "🔨 Building free test executable..."
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_FREE_STANDALONE -o $@ $< $(LIBPATH) $(LIBS)

$(REALLOC_TEST_BIN): test_realloc.c | $(BINDIR)
	@echo "🔨 Building realloc test executable..."
	$(CC) $(CFLAGS) $(INCLUDES) -DTEST_REALLOC_STANDALONE -o $@ $< $(LIBPATH) $(LIBS)

# ================================ COORDINATED TESTS ======================== #
# NASA principle: Integration testing of component interactions

$(RUNNER_BIN): $(OBJDIR)/test_runner.o $(CORE_TEST_OBJS) | $(BINDIR)
	@echo "🔨 Building test runner..."
	$(CC) $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

$(ALL_TEST_BIN): $(OBJDIR)/test_all.o $(CORE_TEST_OBJS) | $(BINDIR)
	@echo "🔨 Building comprehensive test suite..."
	$(CC) $(CFLAGS) -o $@ $^ $(LIBPATH) $(LIBS)

# ================================ OBJECT FILES ============================= #
# NASA principle: Clear build dependencies and modularity

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@echo "⚙️  Compiling test file $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ================================ TEST EXECUTION =========================== #
# NASA principle: Comprehensive validation at multiple levels

# Quick validation tests for development
test-quick: $(ALL_TEST_BIN)
	@echo "\n🚀 Running quick validation tests..."
	@echo "======================================="
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(ALL_TEST_BIN) --quick

# Comprehensive test suite  
test: $(ALL_TEST_BIN)
	@echo "\n🧪 Running comprehensive test suite..."
	@echo "======================================"
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(ALL_TEST_BIN)

# Individual component tests
test-malloc: $(MALLOC_TEST_BIN)
	@echo "\n🔍 Testing malloc() function..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(MALLOC_TEST_BIN)

test-free: $(FREE_TEST_BIN)
	@echo "\n🔍 Testing free() function..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(FREE_TEST_BIN)

test-realloc: $(REALLOC_TEST_BIN)
	@echo "\n🔍 Testing realloc() function..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(REALLOC_TEST_BIN)

# Coordinated test runner
test-runner: $(RUNNER_BIN)
	@echo "\n🎯 Running coordinated test runner..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(RUNNER_BIN)

# Performance validation
test-perf: $(ALL_TEST_BIN)
	@echo "\n⚡ Running performance validation..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(ALL_TEST_BIN) performance

# Stress testing  
test-stress: $(ALL_TEST_BIN)
	@echo "\n💪 Running stress tests..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(ALL_TEST_BIN) stress

# Memory leak detection
test-leaks: $(ALL_TEST_BIN)
	@echo "\n🔍 Running memory leak detection..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $(ALL_TEST_BIN) leaks

# Valgrind analysis (if available)
test-valgrind: $(ALL_TEST_BIN)
	@echo "\n🛡️  Running Valgrind analysis..."
	@if command -v valgrind >/dev/null 2>&1; then \
		LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) valgrind \
		--leak-check=full --show-leak-kinds=all --track-origins=yes \
		--suppressions=/dev/null $(ALL_TEST_BIN) --quick; \
	else \
		echo "❌ Valgrind not available on this system"; \
	fi

# ================================ UTILITY TARGETS ========================== #

# Clean object files
clean:
	@echo "🧹 Cleaning test object files..."
	@rm -rf $(OBJDIR)

# Full clean
fclean: clean
	@echo "🗑️  Removing test binaries..."
	@rm -rf $(BINDIR)

# Rebuild everything
re: fclean all

# Help information
help:
	@echo ""
	@echo "🧪 MALLOC TEST SUITE MAKEFILE"
	@echo "============================="
	@echo ""
	@echo "Build targets:"
	@echo "  all              Build all test executables"
	@echo "  clean            Remove object files"
	@echo "  fclean           Remove all test binaries"
	@echo "  re               Clean and rebuild everything"
	@echo ""
	@echo "Test execution:"
	@echo "  test             Run comprehensive test suite"
	@echo "  test-quick       Run quick validation tests"
	@echo "  test-malloc      Test malloc() function only"
	@echo "  test-free        Test free() function only" 
	@echo "  test-realloc     Test realloc() function only"
	@echo "  test-runner      Run coordinated test runner"
	@echo ""
	@echo "Advanced testing:"
	@echo "  test-perf        Performance validation"
	@echo "  test-stress      Stress testing"
	@echo "  test-leaks       Memory leak detection"
	@echo "  test-valgrind    Valgrind analysis"
	@echo ""
	@echo "Architecture:"
	@echo "  - Individual component tests (NASA modularity)"
	@echo "  - Integration testing"
	@echo "  - Comprehensive validation"
	@echo "  - Performance benchmarking"
	@echo ""
