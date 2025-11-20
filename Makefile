ifeq ($(HOSTTYPE),)
    HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

NAME        = libft_malloc_$(HOSTTYPE).so
LINK_NAME   = libft_malloc.so

CC          = gcc
CFLAGS      = -Wall -Wextra -Werror -fPIC -g3 -std=c99
LDFLAGS     = -shared

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -D LINUX
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -D MACOS
endif

SRCDIR      = src
INCDIR      = include
BUILDDIR    = build
OBJDIR      = $(BUILDDIR)/obj
BINDIR      = $(BUILDDIR)/bin
LIBDIR      = lib
LIBFT_LIB   = $(LIBDIR)/build/libft.a
LIBFT_INC   = $(LIBDIR)/include

CORE_SRCS   = $(SRCDIR)/core/globals.c \
              $(SRCDIR)/core/malloc.c \
              $(SRCDIR)/core/free.c \
              $(SRCDIR)/core/realloc.c

ZONE_SRCS   = $(SRCDIR)/zone/zone.c

CHUNK_SRCS  = $(SRCDIR)/chunk/chunk.c

UTILS_SRCS  = $(SRCDIR)/utils/show_alloc_mem.c \
              $(SRCDIR)/utils/stats.c \
              $(SRCDIR)/utils/cleanup.c \
              $(SRCDIR)/utils/output.c \
              $(SRCDIR)/utils/memory.c

SRCS        = $(CORE_SRCS) $(ZONE_SRCS) $(CHUNK_SRCS) $(UTILS_SRCS)
OBJS        = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

LIBFT_DIR   = $(LIBDIR)
LIBFT       = $(LIBFT_LIB)

.PHONY: all clean fclean re help

all: $(NAME)

$(OBJDIR) $(BINDIR):
	@mkdir -p $@

$(NAME): $(LIBFT) $(OBJS) | $(BINDIR)
	@echo "Creating library $(NAME)..."
	$(CC) $(LDFLAGS) -o $(BINDIR)/$(NAME) $(OBJS) -L$(LIBDIR)/build -lft -lpthread
	@rm -f $(BINDIR)/$(LINK_NAME)
	@ln -s $(NAME) $(BINDIR)/$(LINK_NAME)
	@rm -f $(LINK_NAME)
	@ln -s $(BINDIR)/$(NAME) $(LINK_NAME)
	@echo "$(NAME) created successfully!"
	@echo "HOSTTYPE: $(HOSTTYPE)"
	@echo "Location: $(BINDIR)/$(NAME)"

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBFT_INC) -c $< -o $@

$(LIBFT):
	@echo "Building libft..."
	@$(MAKE) -C $(LIBFT_DIR)

clean:
	@echo "Cleaning object files..."
	@rm -rf $(OBJDIR)
	@$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	@echo "Removing libraries and binaries..."
	@rm -rf $(BUILDDIR)
	@rm -f $(LINK_NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

help:
	@echo ""
	@echo "MALLOC PROJECT MAKEFILE"
	@echo "========================"
	@echo ""
	@echo "Build targets:"
	@echo "  all              Build the malloc library"
	@echo "  clean            Remove object files"
	@echo "  fclean           Remove all generated files"
	@echo "  re               Clean and rebuild everything"
	@echo "  help             Show this help message"
	@echo ""
	@echo "Library info:"
	@echo "  Name: $(NAME)"
	@echo "  HOSTTYPE: $(HOSTTYPE)"
	@echo "  Output: $(BINDIR)/$(NAME)"
	@echo ""
	@echo "Source files:"
	@echo "  Core: $(CORE_SRCS)"
	@echo "  Zone: $(ZONE_SRCS)"
	@echo "  Chunk: $(CHUNK_SRCS)"
	@echo "  Utils: $(UTILS_SRCS)"
	@echo ""
