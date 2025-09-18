#ifndef MALLOC_H
# define MALLOC_H

# include <sys/mman.h>
# include <unistd.h>
# include <pthread.h>
# include <stdlib.h>
# include "includes/libft/libft.h"

/* ===== Gestion multi-plateforme ===== */
# ifdef LINUX
#  define GET_PAGE_SIZE() sysconf(_SC_PAGESIZE)
# elif MACOS
#  define GET_PAGE_SIZE() getpagesize()
# else
#  define GET_PAGE_SIZE() 4096  // Fallback par défaut
# endif

/* ===== Prototypes des fonctions exportées ===== */
void    free(void *ptr);
void    *malloc(size_t size);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

#endif