# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/18 11:17:50 by rluiz             #+#    #+#              #
#    Updated: 2025/09/19 09:41:24 by rluiz            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ================================ HOSTTYPE ================================= #
# Détection automatique de HOSTTYPE si non défini
ifeq ($(HOSTTYPE),)
    HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

# ================================ VARIABLES ================================ #
NAME        = libft_malloc_$(HOSTTYPE).so
LINK_NAME   = libft_malloc.so

CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -fPIC -g3 -std=c99
LDFLAGS     = -shared

# Détection de l'OS pour les différences Linux/Mac
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D LINUX
    # Sur Linux, utiliser sysconf(_SC_PAGESIZE)
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D MACOS
    # Sur Mac, utiliser getpagesize()
endif

# ================================ DIRECTORIES =============================== #
SRCDIR      = src
INCDIR      = include
SRCINTDIR   = $(SRCDIR)/internal
BUILDDIR    = build
OBJDIR      = $(BUILDDIR)/obj
BINDIR      = $(BUILDDIR)/bin
LIBDIR      = lib
LIBFT_LIB   = $(LIBDIR)/build/libft.a
LIBFT_INC   = $(LIBDIR)/include
TESTDIR     = tests
DOCDIR      = docs
EXAMPLEDIR  = examples

# ================================ SOURCES ================================== #
# Core functionality sources (Phase 1: Simple implementation)
CORE_SRCS   = $(SRCDIR)/core/malloc.c \
              $(SRCDIR)/core/free.c \
              $(SRCDIR)/core/realloc.c

# Utility sources  
UTILS_SRCS  = $(SRCDIR)/utils/show_alloc_mem.c \
              $(SRCDIR)/utils/memory_utils.c \
              $(SRCDIR)/utils/debug.c

# Phase 1 sources (minimal implementation)
SRCS        = $(CORE_SRCS) $(UTILS_SRCS)

# Phase 2+ sources (will be added later)
# WRAPPER_SRCS = $(SRCDIR)/wrappers/mmap_wrapper.c \
#                $(SRCDIR)/wrappers/error_handler.c
# ZONE_SRCS   = $(SRCDIR)/zones/zone_manager.c
# INTERNAL_SRCS = $(SRCDIR)/internal/chunk_manager.c

OBJS        = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# ================================ LIBFT ==================================== #
LIBFT_DIR   = $(LIBDIR)
LIBFT       = $(LIBFT_LIB)

# ================================ RULES ==================================== #
.PHONY: all clean fclean re test test-quick test-all test-performance test-stress examples docs install help

all: $(NAME)

# Create build directories
$(OBJDIR) $(BINDIR):
	@mkdir -p $@

# Compilation de la bibliothèque dynamique
$(NAME): $(LIBFT) $(OBJS) | $(BINDIR)
	@echo "📦 Creating library $(NAME)..."
	$(CC) $(LDFLAGS) -o $(BINDIR)/$(NAME) $(OBJS) -L$(LIBDIR)/build -lft
	@echo "🔗 Creating symbolic link $(LINK_NAME) -> $(NAME)"
	@rm -f $(BINDIR)/$(LINK_NAME)
	@ln -s $(NAME) $(BINDIR)/$(LINK_NAME)
	@rm -f $(LINK_NAME)
	@ln -s $(BINDIR)/$(NAME) $(LINK_NAME)
	@echo "✅ $(NAME) created successfully!"
	@echo "   HOSTTYPE: $(HOSTTYPE)"
	@echo "   Location: $(BINDIR)/$(NAME)"

# Compilation des objets avec structure NASA
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	@echo "⚙️  Compiling $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBFT_INC) -I$(SRCINTDIR) -c $< -o $@

# Compilation de la libft
$(LIBFT):
	@echo "📚 Building libft..."
	@$(MAKE) -C $(LIBFT_DIR)

# ================================ TESTING ================================== #
# Build tests (ensures library is built first)
test-build: $(NAME)
	@echo "🔨 Building test suite..."
	@$(MAKE) -C $(TESTDIR) all

# Quick tests for development
test-quick: test-build
	@echo "\n🚀 Running quick test suite..."
	@$(MAKE) -C $(TESTDIR) test-quick

# Comprehensive test suite
test: test-build
	@echo "\n🧪 Running comprehensive test suite..."
	@$(MAKE) -C $(TESTDIR) test

# All tests (alias for test)
test-all: test

# Performance benchmarks
test-performance: test-build
	@echo "\n⚡ Running performance tests..."
	@$(MAKE) -C $(TESTDIR) test-perf

# Stress testing
test-stress: test-build
	@echo "\n💪 Running stress tests..."
	@$(MAKE) -C $(TESTDIR) test-stress

# Memory leak testing
test-leaks: test-build
	@echo "\n🔍 Running memory leak tests..."
	@$(MAKE) -C $(TESTDIR) test-leaks

# Individual test suites
test-malloc: test-build
	@$(MAKE) -C $(TESTDIR) test-malloc

test-free: test-build
	@$(MAKE) -C $(TESTDIR) test-free

test-realloc: test-build
	@$(MAKE) -C $(TESTDIR) test-realloc

# Valgrind testing (if available)
test-valgrind: test-build
	@echo "\n🛡️  Running Valgrind tests..."
	@$(MAKE) -C $(TESTDIR) test-valgrind 2>/dev/null || echo "Valgrind not available"

# ================================ EXAMPLES ================================= #
examples: $(NAME)
	@echo "� Building examples..."
	@$(MAKE) -C $(EXAMPLEDIR) all 2>/dev/null || echo "No examples Makefile found"

# ================================ DOCUMENTATION ============================ #
docs:
	@echo "📚 Generating documentation..."
	@echo "API documentation: $(DOCDIR)/api.md"
	@echo "Design documentation: $(DOCDIR)/design.md"

# ================================ INSTALLATION ============================= #
install: $(NAME)
	@echo "📦 Installing malloc library..."
	@sudo cp $(BINDIR)/$(NAME) /usr/local/lib/
	@sudo cp $(BINDIR)/$(LINK_NAME) /usr/local/lib/
	@sudo cp $(INCDIR)/malloc.h /usr/local/include/
	@sudo ldconfig 2>/dev/null || true
	@echo "✅ Installation complete"

uninstall:
	@echo "🗑️  Uninstalling malloc library..."
	@sudo rm -f /usr/local/lib/$(NAME)
	@sudo rm -f /usr/local/lib/$(LINK_NAME)
	@sudo rm -f /usr/local/include/malloc.h
	@sudo ldconfig 2>/dev/null || true
	@echo "✅ Uninstallation complete"

# ================================ CLEANUP =================================== #
clean:
	@echo "🧹 Cleaning object files..."
	@rm -rf $(OBJDIR)
	@$(MAKE) -C $(LIBFT_DIR) clean
	@$(MAKE) -C $(TESTDIR) clean 2>/dev/null || true
	@$(MAKE) -C $(EXAMPLEDIR) clean 2>/dev/null || true

fclean: clean
	@echo "🗑️  Removing libraries and binaries..."
	@rm -rf $(BUILDDIR)
	@rm -f $(LINK_NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean
	@$(MAKE) -C $(TESTDIR) fclean 2>/dev/null || true
	@$(MAKE) -C $(EXAMPLEDIR) fclean 2>/dev/null || true

re: fclean all

# ================================ HELP ===================================== #
help:
	@echo ""
	@echo "🏗️  MALLOC PROJECT MAKEFILE"
	@echo "=========================="
	@echo ""
	@echo "Build targets:"
	@echo "  all              Build the malloc library"
	@echo "  clean            Remove object files"
	@echo "  fclean           Remove all generated files"
	@echo "  re               Clean and rebuild everything"
	@echo ""
	@echo "Testing targets:"
	@echo "  test             Run comprehensive test suite"
	@echo "  test-quick       Run quick tests for development"
	@echo "  test-performance Run performance benchmarks"
	@echo "  test-stress      Run stress tests"
	@echo "  test-leaks       Run memory leak tests"
	@echo "  test-valgrind    Run tests with Valgrind"
	@echo "  test-malloc      Run malloc tests only"
	@echo "  test-free        Run free tests only"
	@echo "  test-realloc     Run realloc tests only"
	@echo ""
	@echo "Other targets:"
	@echo "  examples         Build example programs"
	@echo "  docs             Show documentation info"
	@echo "  install          Install library system-wide"
	@echo "  uninstall        Remove system-wide installation"
	@echo "  help             Show this help message"
	@echo ""
	@echo "Library info:"
	@echo "  Name: $(NAME)"
	@echo "  HOSTTYPE: $(HOSTTYPE)"
	@echo "  Output: $(BINDIR)/$(NAME)"
	@echo ""