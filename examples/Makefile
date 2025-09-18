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
INCDIR = ../include
BUILDDIR = ../build
BINDIR = $(BUILDDIR)/bin
EXAMPLEDIR = .

# Library paths
MALLOC_LIB_DIR = $(BUILDDIR)/bin
LIBFT_DIR = ../lib

# Include and library paths
INCLUDES = -I$(INCDIR) -I$(LIBFT_DIR)
LIBPATH = -L$(MALLOC_LIB_DIR) -L$(LIBFT_DIR)
LIBS = -lft_malloc -lft

# ================================ EXAMPLES ================================== #
EXAMPLES = basic_usage advanced_usage performance_demo

EXAMPLE_BINS = $(EXAMPLES:%=$(BINDIR)/example_%)

# ================================ BUILD RULES ============================== #
.PHONY: all clean help

all: $(EXAMPLE_BINS)

$(BINDIR)/example_%: $(EXAMPLEDIR)/%.c | $(BINDIR)
	@echo "🔨 Building example: $*"
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBPATH) $(LIBS)

$(BINDIR):
	@mkdir -p $@

# ================================ RUN EXAMPLES ============================= #
.PHONY: run-basic run-advanced run-performance run-all

run-basic: $(BINDIR)/example_basic_usage
	@echo "🚀 Running basic usage example..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $<

run-advanced: $(BINDIR)/example_advanced_usage
	@echo "🚀 Running advanced usage example..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $<

run-performance: $(BINDIR)/example_performance_demo
	@echo "🚀 Running performance demo..."
	@LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $<

run-all: $(EXAMPLE_BINS)
	@echo "🚀 Running all examples..."
	@for example in $(EXAMPLE_BINS); do \
		echo ""; \
		echo "Running $$example..."; \
		LD_LIBRARY_PATH=$(MALLOC_LIB_DIR):$(LIBFT_DIR) $$example; \
	done

# ================================ CLEANUP =================================== #
clean:
	@echo "🧹 Cleaning example binaries..."
	@rm -f $(EXAMPLE_BINS)

# ================================ HELP ===================================== #
help:
	@echo ""
	@echo "📋 MALLOC EXAMPLES MAKEFILE"
	@echo "==========================="
	@echo ""
	@echo "Build targets:"
	@echo "  all              Build all examples"
	@echo "  clean            Remove example binaries"
	@echo ""
	@echo "Run targets:"
	@echo "  run-basic        Run basic usage example"
	@echo "  run-advanced     Run advanced usage example"  
	@echo "  run-performance  Run performance demo"
	@echo "  run-all          Run all examples"
	@echo ""
	@echo "Available examples:"
	@echo "  basic_usage      Simple malloc/free demonstration"
	@echo "  advanced_usage   Complex allocation patterns"
	@echo "  performance_demo Performance comparison"
	@echo ""
