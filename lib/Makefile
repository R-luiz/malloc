# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rluiz <marvin@42.fr>                       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/04/16 15:00:46 by rluiz             #+#    #+#              #
#    Updated: 2025/09/18 XX:XX:XX by rluiz            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# ================================ NASA ARCHITECTURE ======================== #
# This Makefile follows NASA C coding standards (JPL/NASA-STD-1003)
# - Clear separation of source, include, and build directories
# - Explicit dependency management
# - Consistent naming conventions

# ================================ PROJECT CONFIGURATION ==================== #
NAME = libft.a

# Compiler configuration
CC = cc
CFLAGS = -Wall -Werror -Wextra -std=c99

# Directory structure (NASA principle: organized file hierarchy)
SRCDIR = src
INCDIR = include
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj

# Include paths
INCLUDES = -I$(INCDIR)

# ================================ SOURCE FILES ============================= #
# NASA principle: Explicit and organized source management

SRCS  = ft_isalpha.c ft_isdigit.c ft_isalnum.c ft_isascii.c ft_isprint.c \
       ft_strlen.c ft_memset.c ft_bzero.c ft_memcpy.c ft_memmove.c \
       ft_strlcpy.c ft_strlcat.c ft_toupper.c ft_tolower.c ft_strchr.c \
       ft_strrchr.c ft_strncmp.c ft_memchr.c ft_memcmp.c ft_strnstr.c \
       ft_atoi.c ft_calloc.c ft_strdup.c ft_substr.c ft_strjoin.c \
       ft_strtrim.c ft_split.c ft_itoa.c ft_strmapi.c ft_putchar_fd.c \
       ft_putstr_fd.c ft_putendl_fd.c ft_putnbr_fd.c ft_striteri.c

BONUS_SRCS = ft_lstnew.c ft_lstadd_front.c ft_lstsize.c \
             ft_lstlast.c ft_lstadd_back.c ft_lstdelone.c \
             ft_lstclear.c ft_lstiter.c ft_lstmap.c

# Object files with proper paths
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
BONUS_OBJS = $(BONUS_SRCS:%.c=$(OBJDIR)/%.o)

# Target library with build directory
TARGET = $(BUILDDIR)/$(NAME)

# ================================ BUILD TARGETS =========================== #
# NASA principle: Clear and predictable build process

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILDDIR)
	@echo "📚 Creating libft library..."
	ar rcs $@ $(OBJS)
	@echo "✅ $(NAME) created successfully!"

bonus: $(TARGET) $(BONUS_OBJS)
	@echo "📚 Adding bonus functions to libft..."
	ar rcs $(TARGET) $(BONUS_OBJS)
	@echo "✅ Bonus functions added!"

# ================================ OBJECT COMPILATION ====================== #
# NASA principle: Consistent compilation with proper dependencies

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@echo "⚙️  Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ================================ DIRECTORY CREATION ====================== #
# NASA principle: Proper build environment setup

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# ================================ MAINTENANCE TARGETS ===================== #
# NASA principle: Clean and maintainable build environment

clean:
	@echo "🧹 Cleaning libft object files..."
	rm -f $(OBJDIR)/*.o

fclean: clean
	@echo "🧹 Cleaning libft library..."
	rm -f $(TARGET)
	@rmdir $(OBJDIR) 2>/dev/null || true
	@rmdir $(BUILDDIR) 2>/dev/null || true

re: fclean all

# ================================ UTILITY TARGETS ========================== #
# NASA principle: Additional development support

install: $(TARGET)
	@echo "📦 Installing libft to parent project..."
	@cp $(TARGET) ../libft.a || echo "⚠️  Could not install to parent directory"

.PHONY: all bonus clean fclean re install
