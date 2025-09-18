# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/18 11:17:50 by rluiz             #+#    #+#              #
#    Updated: 2025/09/18 11:17:51 by rluiz            ###   ########.fr        #
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
CFLAGS      = -Wall -Wextra -Werror -fPIC -g
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

# ================================ SOURCES ================================== #
SRCS_DIR    = src/
OBJS_DIR    = obj/
INC_DIR     = includes/

SRCS        = malloc.c \
              free.c \
              realloc.c \
              show_alloc_mem.c \
              zone_manager.c \
              mmap_handler.c \
              utils.c

OBJS        = $(addprefix $(OBJS_DIR), $(SRCS:.c=.o))

# ================================ LIBFT ==================================== #
LIBFT_DIR   = libft/
LIBFT       = $(LIBFT_DIR)libft.a

# ================================ RULES ==================================== #
all: $(NAME)

# Compilation de la bibliothèque dynamique
$(NAME): $(LIBFT) $(OBJS)
	@echo "📦 Creating library $(NAME)..."
	$(CC) $(LDFLAGS) -o $(NAME) $(OBJS) -L$(LIBFT_DIR) -lft
	@echo "🔗 Creating symbolic link $(LINK_NAME) -> $(NAME)"
	@rm -f $(LINK_NAME)
	@ln -s $(NAME) $(LINK_NAME)
	@echo "✅ $(NAME) created successfully!"
	@echo "   HOSTTYPE: $(HOSTTYPE)"

# Compilation des objets
$(OBJS_DIR)%.o: $(SRCS_DIR)%.c | $(OBJS_DIR)
	@echo "⚙️  Compiling $<..."
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(LIBFT_DIR) -c $< -o $@

# Création du dossier obj
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# Compilation de la libft
$(LIBFT):
	@echo "📚 Building libft..."
	@$(MAKE) -C $(LIBFT_DIR)

# Nettoyage
clean:
	@echo "🧹 Cleaning object files..."
	@rm -rf $(OBJS_DIR)
	@$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	@echo "🗑️  Removing libraries..."
	@rm -f $(NAME) $(LINK_NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

# Tests pour vérifier la bibliothèque
test: all
	@echo "\n🧪 Running tests..."
	@echo "Library info:"
	@ls -la | grep libft_malloc
	@echo "\nHOSTTYPE: $(HOSTTYPE)"
	@echo "Library name: $(NAME)"
	@echo "\nTesting dynamic loading:"
	$(CC) -o test_malloc tests/main.c -L. -lft_malloc
	./test_malloc

.PHONY: all clean fclean re test