/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 09:38:34 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_H
# define MALLOC_H

# include <sys/mman.h>
# include <unistd.h>
# include <pthread.h>
# include <stdlib.h>
# include <stddef.h>
# include <stdint.h>

/*
** NASA C Standard Compliance:
** - Clear public API
** - Platform-specific handling
** - Thread-safe operations
** - Bounded allocations
*/

/* ===== Platform-specific page size handling ===== */
# ifdef LINUX
#  define GET_PAGE_SIZE() sysconf(_SC_PAGESIZE)
# elif MACOS
#  define GET_PAGE_SIZE() getpagesize()
# else
#  define GET_PAGE_SIZE() 4096  // Fallback default
# endif

/* ===== Public Function Prototypes ===== */
/*
** Standard memory allocation functions
** NASA: Thread-safe, validated, bounded operations
*/
void    free(void *ptr);
void    *malloc(size_t size);
void    *realloc(void *ptr, size_t size);

/*
** Debug and monitoring functions
** NASA: Diagnostic capabilities for validation
*/
void    show_alloc_mem(void);

/*
** System validation and statistics
** NASA: Runtime system integrity checking
*/
int     malloc_validate_system(void);

/* ===== Public Type Definitions ===== */
/*
** Statistics structure for monitoring
** NASA: Bounded data collection for monitoring
*/
typedef struct s_malloc_stats {
    size_t          bytes_allocated;    /* Current allocated bytes */
    size_t          bytes_peak;         /* Peak allocation */
    size_t          bytes_total;        /* Lifetime allocated */
    
    uint32_t        allocs_tiny;        /* Tiny allocations count */
    uint32_t        allocs_small;       /* Small allocations count */
    uint32_t        allocs_large;       /* Large allocations count */
    
    uint32_t        zones_active;       /* Currently active zones */
    uint32_t        zones_total;        /* Total zones created */
    
    uint32_t        errors_count;       /* Error count */
    uint32_t        corruption_count;   /* Corruption detections */
    
    double          fragmentation;      /* Fragmentation ratio */
    uint64_t        update_time;        /* Last update timestamp */
} t_malloc_stats;

/*
** Get allocation statistics
** Pre-conditions: stats != NULL
** Post-conditions: stats filled with current values
** NASA: Bounded data collection for monitoring
*/
int     get_malloc_stats(t_malloc_stats *stats);

/*
** Check for memory leaks
** Returns: 0 if no leaks, positive number of leaks detected
** NASA: Resource leak detection
*/
int     check_malloc_leaks(void);

#endif