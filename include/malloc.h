#ifndef MALLOC_H
# define MALLOC_H

# include <sys/mman.h>
# include <unistd.h>
# include <pthread.h>
# include <stdlib.h>
# include <stddef.h>
# include <stdint.h>

# ifdef LINUX
#  define GET_PAGE_SIZE() sysconf(_SC_PAGESIZE)
# elif MACOS
#  define GET_PAGE_SIZE() getpagesize()
# else
#  define GET_PAGE_SIZE() 4096
# endif

void    free(void *ptr);
void    *malloc(size_t size);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem(void);
int     malloc_validate_system(void);

typedef struct s_malloc_stats {
    size_t          bytes_allocated;
    size_t          bytes_peak;
    size_t          bytes_total;
    uint32_t        allocs_tiny;
    uint32_t        allocs_small;
    uint32_t        allocs_large;
    uint32_t        zones_active;
    uint32_t        zones_total;
    uint32_t        errors_count;
    uint32_t        corruption_count;
    double          fragmentation;
    uint64_t        update_time;
} t_malloc_stats;

int     get_malloc_stats(t_malloc_stats *stats);
int     check_malloc_leaks(void);

#endif
