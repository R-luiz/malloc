/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 10:00:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 10:42:07 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/malloc.h"
#include "../../include/types.h"
#include "../internal/zone_manager.h"
#include "../internal/metadata.h"
#include "../internal/malloc_constants.h"
#include "../wrappers/mmap_wrapper.h"
#include "../wrappers/error_handler.h"
#include <pthread.h>
#include <assert.h>
#include <string.h>

/*
** NASA C Standard Compliance:
** - Zone-based memory management with TINY/SMALL/LARGE categories
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - Thread-safety with pthread_mutex
** - Bounded operations with iteration limits
*/

/* ===== External Declarations ===== */
pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;
t_malloc_stats g_malloc_stats = {0};

/* ===== External Function Declarations ===== */
extern t_chunk *chunk_split(t_chunk *chunk, size_t size);

/* ===== Helper Functions ===== */

/*
** Ensure zone manager is initialized (singleton pattern)
** NASA: One-time initialization with error handling
*/
static int ensure_initialized(void)
{
    static int initialized = 0;
    if (!initialized) {
        if (mmap_wrapper_init() != 0) {
            handle_error(ERROR_MMAP_FAILED, "Failed to initialize mmap wrapper");
            return -1;
        }
        if (zone_manager_global_init() != 0) {
            handle_error(ERROR_ZONE_CREATION, "Failed to initialize zone manager");
            return -1;
        }
        initialized = 1;
    }
    return 0;
}

/*
** Allocate from TINY or SMALL zone
** NASA: Zone-based allocation with reuse
*/
static void *allocate_in_zone(t_zone_manager *manager, zone_type_t zone_type, size_t size)
{
    /* Find existing zone with free space */
    t_zone *zone = manager_find_zone_for_size(manager, size);
    
    /* Create new zone if needed */
    if (!zone) {
        size_t zone_size = (zone_type == ZONE_TINY) ? TINY_ZONE_SIZE : SMALL_ZONE_SIZE;
        zone = zone_create(zone_type, zone_size);
        if (!zone) {
            return NULL;
        }
        manager_add_zone(manager, zone);
    }
    
    /* Find or split free chunk */
    t_chunk *chunk = zone_find_free_chunk(zone, size);
    if (!chunk) {
        return NULL;
    }
    
    /* Split chunk if too large */
    if (chunk->size > size + MIN_CHUNK_SIZE) {
        chunk_split(chunk, size);
    }
    
    /* Mark chunk as allocated */
    chunk_set_state(chunk, CHUNK_ALLOCATED);
    
    /* Return user pointer (after chunk header) */
    return (char *)chunk + CHUNK_HEADER_SIZE;
}

/*
** Allocate LARGE allocation with direct mmap
** NASA: Direct allocation for large sizes
*/
static void *allocate_large(size_t size)
{
    /* Calculate total size including headers */
    size_t total_size = ALIGN_UP(size + ZONE_HEADER_SIZE + CHUNK_HEADER_SIZE);
    
    /* Direct mmap for large allocation */
    void *addr = safe_mmap(total_size);
    if (!addr) {
        return NULL;
    }
    
    /* Create zone and chunk structures */
    t_zone *zone = zone_create(ZONE_LARGE, total_size);
    if (!zone) {
        safe_munmap(addr, total_size);
        return NULL;
    }
    
    /* Initialize zone at mapped address */
    memcpy(addr, zone, ZONE_HEADER_SIZE);
    zone = (t_zone *)addr;
    zone->start_addr = addr;
    zone->end_addr = (char *)addr + total_size;
    
    /* Create single chunk for the entire allocation */
    void *chunk_addr = (char *)addr + ZONE_HEADER_SIZE;
    t_chunk *chunk = chunk_create(chunk_addr, size, zone);
    if (!chunk) {
        safe_munmap(addr, total_size);
        return NULL;
    }
    
    chunk_set_state(chunk, CHUNK_ALLOCATED);
    zone->first_chunk = chunk;
    
    return (char *)chunk + CHUNK_HEADER_SIZE;
}

/*
** Main malloc implementation with zone-based allocation
** NASA: Bounded operation with comprehensive validation
*/
void *malloc(size_t size)
{
    /* Validate input parameters */
    if (size == 0 || size > MAX_ALLOC_SIZE) {
        return NULL;
    }
    
    /* Thread safety */
    pthread_mutex_lock(&g_malloc_mutex);
    
    /* Ensure system is initialized */
    if (ensure_initialized() != 0) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;
    }
    
    /* Get global zone manager */
    t_zone_manager *manager = get_global_zone_manager();
    if (!manager) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;
    }
    
    /* Determine zone type and align size */
    zone_type_t zone_type = get_zone_type_for_size(size);
    size_t aligned_size = ALIGN_UP(size);
    
    void *result = NULL;
    
    /* Handle allocation based on zone type */
    if (zone_type == ZONE_LARGE) {
        result = allocate_large(aligned_size);
    } else {
        result = allocate_in_zone(manager, zone_type, aligned_size);
    }
    
    /* Update statistics */
    if (result) {
        g_malloc_stats.bytes_allocated += aligned_size;
        g_malloc_stats.bytes_total += aligned_size;
        if (g_malloc_stats.bytes_allocated > g_malloc_stats.bytes_peak) {
            g_malloc_stats.bytes_peak = g_malloc_stats.bytes_allocated;
        }
        
        /* Categorize allocation */
        if (zone_type == ZONE_TINY) {
            g_malloc_stats.allocs_tiny++;
        } else if (zone_type == ZONE_SMALL) {
            g_malloc_stats.allocs_small++;
        } else {
            g_malloc_stats.allocs_large++;
        }
    }
    
    pthread_mutex_unlock(&g_malloc_mutex);
    return result;
}
