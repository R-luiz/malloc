/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:38 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/malloc.h"
#include "../internal/metadata.h"
#include "../internal/zone_manager.h"
#include "../wrappers/mmap_wrapper.h"
#include "../wrappers/error_handler.h"
#include <pthread.h>
#include <assert.h>
#include <string.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - No recursion
** - Thread-safety with pthread_mutex
** - Assert invariants and pre/post conditions
** - Dispatch logic based on size (TINY/SMALL/LARGE)
** - Metadata structure with magic numbers for corruption detection
*/

/* ===== Global State with Thread Safety ===== */
static pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_malloc_initialized = 0;
static t_malloc_stats g_malloc_stats = {0};

/* ===== External Function Declarations ===== */
extern t_chunk *chunk_create(void *addr, size_t size, t_zone *zone);
extern int chunk_validate(const t_chunk *chunk);
extern int chunk_set_state(t_chunk *chunk, chunk_state_t state);
extern void *chunk_get_data_ptr(const t_chunk *chunk);
extern t_chunk *chunk_from_data_ptr(void *data_ptr);
extern t_chunk *chunk_split(t_chunk *chunk, size_t size);

/* ===== Helper Functions ===== */

/*
** Initialize malloc system once
** NASA: One-time system initialization with validation
*/
static int malloc_system_init(void)
{
    if (g_malloc_initialized) {
        return 0; /* Already initialized */
    }
    
    /* Initialize error handling */
    if (error_handler_init() != 0) {
        return -1;
    }
    
    /* Initialize mmap wrapper */
    if (mmap_wrapper_init() != 0) {
        handle_error(ERROR_INITIALIZATION, "Failed to initialize mmap wrapper");
        return -1;
    }
    
    /* Initialize zone manager */
    if (zone_manager_global_init() != 0) {
        handle_error(ERROR_INITIALIZATION, "Failed to initialize zone manager");
        return -1;
    }
    
    /* Initialize statistics */
    memset(&g_malloc_stats, 0, sizeof(g_malloc_stats));
    g_malloc_stats.update_time = get_timestamp_us();
    
    g_malloc_initialized = 1;
    return 0;
}

/*
** Validate malloc parameters
** NASA: Comprehensive parameter validation
*/
static int validate_malloc_params(size_t size)
{
    if (size == 0) {
        handle_error(ERROR_INVALID_SIZE, "malloc called with size 0");
        return 0;
    }
    
    if (size > MAX_ALLOC_SIZE) {
        handle_error(ERROR_SIZE_LIMIT_EXCEEDED, "Allocation size exceeds limit");
        return 0;
    }
    
    return 1;
}

/*
** Allocate from existing zone or create new one
** NASA: Bounded operation with fallback strategy
*/
static void *allocate_from_zone(size_t size, zone_type_t type)
{
    t_zone_manager *manager = get_global_zone_manager();
    if (!manager) {
        handle_error(ERROR_INTERNAL_STATE, "Zone manager not available");
        return NULL;
    }
    
    /* Try to find existing zone with free space */
    t_zone *zone = manager_find_zone_for_size(manager, size);
    
    /* If no suitable zone found, create new one */
    if (!zone) {
        zone = zone_create(type, size);
        if (!zone) {
            handle_error(ERROR_ZONE_CREATION, "Failed to create new zone");
            return NULL;
        }
        
        /* Add zone to manager */
        if (manager_add_zone(manager, zone) != 0) {
            handle_error(ERROR_INTERNAL_STATE, "Failed to add zone to manager");
            return NULL;
        }
    }
    
    /* Find free chunk in zone */
    t_chunk *chunk = zone_find_free_chunk(zone, size);
    if (!chunk) {
        handle_error(ERROR_CHUNK_NOT_FOUND, "No suitable chunk found in zone");
        return NULL;
    }
    
    /* Split chunk if necessary */
    chunk = chunk_split(chunk, size);
    if (!chunk) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Failed to split chunk");
        return NULL;
    }
    
    /* Mark chunk as allocated */
    if (chunk_set_state(chunk, CHUNK_ALLOCATED) != 0) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Failed to set chunk state");
        return NULL;
    }
    
    /* Update zone statistics */
    zone->used_size += chunk_get_total_size(chunk);
    if (chunk->state == CHUNK_FREE) {
        zone->free_count--;
    }
    
    return chunk_get_data_ptr(chunk);
}

/*
** Allocate large memory directly with mmap
** NASA: Direct allocation for large sizes
*/
static void *allocate_large(size_t size)
{
    /* Create single-chunk zone for large allocation */
    t_zone *zone = zone_create(ZONE_LARGE, size);
    if (!zone) {
        handle_error(ERROR_ZONE_CREATION, "Failed to create large zone");
        return NULL;
    }
    
    /* Add zone to manager */
    t_zone_manager *manager = get_global_zone_manager();
    if (!manager || manager_add_zone(manager, zone) != 0) {
        handle_error(ERROR_INTERNAL_STATE, "Failed to add large zone to manager");
        return NULL;
    }
    
    /* Create chunk for entire zone data area */
    void *data_start = zone->start_addr;
    size_t data_size = zone->total_size;
    
    t_chunk *chunk = chunk_create(data_start, data_size, zone);
    if (!chunk) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Failed to create large chunk");
        return NULL;
    }
    
    /* Mark as allocated */
    if (chunk_set_state(chunk, CHUNK_ALLOCATED) != 0) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Failed to set large chunk state");
        return NULL;
    }
    
    zone->first_chunk = chunk;
    zone->chunk_count = 1;
    zone->used_size = chunk_get_total_size(chunk);
    
    return chunk_get_data_ptr(chunk);
}

/*
** Update allocation statistics
** NASA: Bounded statistics collection
*/
static void update_alloc_stats(size_t size, zone_type_t type)
{
    g_malloc_stats.bytes_allocated += size;
    g_malloc_stats.allocs_total++;
    
    switch (type) {
        case ZONE_TINY:
            g_malloc_stats.allocs_tiny++;
            break;
        case ZONE_SMALL:
            g_malloc_stats.allocs_small++;
            break;
        case ZONE_LARGE:
            g_malloc_stats.allocs_large++;
            break;
        default:
            break;
    }
    
    if (g_malloc_stats.bytes_allocated > g_malloc_stats.bytes_peak) {
        g_malloc_stats.bytes_peak = g_malloc_stats.bytes_allocated;
    }
    
    g_malloc_stats.update_time = get_timestamp_us();
}

/* ===== Public API Implementation ===== */

/*
** Main malloc implementation
** NASA: Thread-safe allocation with size-based dispatch
*/
void *malloc(size_t size)
{
    /* Thread safety */
    if (pthread_mutex_lock(&g_malloc_mutex) != 0) {
        handle_error(ERROR_THREAD_SAFETY, "Failed to acquire malloc mutex");
        return NULL;
    }
    
    void *result = NULL;
    
    /* Initialize system if needed */
    if (!g_malloc_initialized) {
        if (malloc_system_init() != 0) {
            goto cleanup;
        }
    }
    
    /* Validate parameters */
    if (!validate_malloc_params(size)) {
        goto cleanup;
    }
    
    /* Determine allocation strategy based on size */
    zone_type_t type = get_zone_type_for_size(size);
    size_t aligned_size = ALIGN_UP(size);
    
    /* Dispatch to appropriate allocation function */
    switch (type) {
        case ZONE_TINY:
        case ZONE_SMALL:
            result = allocate_from_zone(aligned_size, type);
            break;
            
        case ZONE_LARGE:
            result = allocate_large(aligned_size);
            break;
            
        default:
            handle_error(ERROR_INVALID_ZONE_TYPE, "Invalid zone type");
            result = NULL;
            break;
    }
    
    /* Update statistics on successful allocation */
    if (result) {
        update_alloc_stats(aligned_size, type);
    }
    
cleanup:
    /* Release mutex */
    if (pthread_mutex_unlock(&g_malloc_mutex) != 0) {
        handle_error(ERROR_THREAD_SAFETY, "Failed to release malloc mutex");
        /* Don't change result - mutex error doesn't affect allocation */
    }
    
    return result;
}

/*
** Get current allocation statistics
** NASA: Statistics reporting with thread safety
*/
int get_malloc_stats(t_malloc_stats *stats)
{
    if (!stats) {
        handle_error(ERROR_NULL_PARAMETER, "NULL stats parameter");
        return -1;
    }
    
    if (pthread_mutex_lock(&g_malloc_mutex) != 0) {
        handle_error(ERROR_THREAD_SAFETY, "Failed to acquire stats mutex");
        return -1;
    }
    
    memcpy(stats, &g_malloc_stats, sizeof(g_malloc_stats));
    
    if (pthread_mutex_unlock(&g_malloc_mutex) != 0) {
        handle_error(ERROR_THREAD_SAFETY, "Failed to release stats mutex");
        return -1;
    }
    
    return 0;
}

/*
** Check malloc system integrity
** NASA: Runtime system validation
*/
int malloc_validate_system(void)
{
    if (pthread_mutex_lock(&g_malloc_mutex) != 0) {
        return 0; /* Cannot validate if can't lock */
    }
    
    int result = 1; /* Assume valid */
    
    if (!g_malloc_initialized) {
        result = 0;
        goto cleanup;
    }
    
    /* Validate zone manager */
    t_zone_manager *manager = get_global_zone_manager();
    if (!manager || !manager_validate(manager)) {
        result = 0;
        goto cleanup;
    }
    
    /* Validate subsystems */
    if (!validate_mmap_wrapper_state()) {
        result = 0;
        goto cleanup;
    }
    
    if (!validate_error_handler_state()) {
        result = 0;
        goto cleanup;
    }
    
cleanup:
    pthread_mutex_unlock(&g_malloc_mutex);
    return result;
}

