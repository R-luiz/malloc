/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   realloc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 10:00:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 10:42:07 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/malloc.h"
#include "../internal/metadata.h"
#include "../internal/zone_manager.h"
#include "../internal/malloc_constants.h"
#include "../wrappers/error_handler.h"
#include <pthread.h>
#include <string.h>
#include <assert.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - Thread-safety with pthread_mutex
** - Zone-aware reallocation logic
** - Data preservation during resize
** - In-place expansion when possible
*/

/* ===== External Declarations ===== */
extern pthread_mutex_t g_malloc_mutex;
extern t_malloc_stats g_malloc_stats;

/* ===== External Function Declarations ===== */
extern int chunk_merge_with_next(t_chunk *chunk);
extern t_chunk *chunk_split(t_chunk *chunk, size_t size);

/* ===== Helper Functions ===== */

/*
** Get chunk from user pointer
** NASA: Pointer validation and conversion
*/
static t_chunk *get_chunk_from_ptr(void *ptr)
{
    if (!ptr) {
        return NULL;
    }
    
    t_chunk *chunk = (t_chunk *)((char *)ptr - CHUNK_HEADER_SIZE);
    
    if (chunk->magic != MAGIC_ALLOC) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Invalid chunk magic number in realloc");
        return NULL;
    }
    
    return chunk;
}

/*
** Try to expand chunk in place
** NASA: In-place expansion for efficiency
*/
static int try_expand_in_place(t_chunk *chunk, size_t new_size)
{
    if (!chunk || !chunk->zone) {
        return -1;
    }
    
    /* Check if current chunk is large enough */
    if (chunk->size >= new_size) {
        /* Can shrink or stay same size */
        return 0;
    }
    
    /* For LARGE allocations, cannot expand in place */
    if (chunk->zone->type == ZONE_LARGE) {
        return -1;
    }
    
    /* Check if next chunk is free and large enough */
    t_chunk *next = chunk->next;
    if (!next || next->state != CHUNK_FREE) {
        return -1;
    }
    
    size_t available = chunk->size + next->size + CHUNK_HEADER_SIZE;
    if (available < new_size) {
        return -1;
    }
    
    /* Merge with next chunk */
    if (chunk_merge_with_next(chunk) != 0) {
        return -1;
    }
    
    /* Split if too large */
    if (chunk->size > new_size + MIN_CHUNK_SIZE) {
        chunk_split(chunk, new_size);
    }
    
    return 0;
}

/*
** Copy data to new allocation
** NASA: Safe data copying with bounds checking
*/
static void *copy_data_to_new_allocation(void *old_ptr, size_t old_size, size_t new_size)
{
    void *new_ptr = malloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    /* Copy minimum of old and new size */
    size_t copy_size = (old_size < new_size) ? old_size : new_size;
    memcpy(new_ptr, old_ptr, copy_size);
    
    /* Free old allocation */
    free(old_ptr);
    
    return new_ptr;
}

/*
** Main realloc implementation
** NASA: Zone-aware reallocation with comprehensive validation
*/
void *realloc(void *ptr, size_t size)
{
    /* Handle special cases */
    if (!ptr) {
        /* realloc(NULL, size) == malloc(size) */
        return malloc(size);
    }
    
    if (size == 0) {
        /* realloc(ptr, 0) == free(ptr) + return NULL */
        free(ptr);
        return NULL;
    }
    
    /* Validate size */
    if (size > MAX_ALLOC_SIZE) {
        return NULL;
    }
    
    /* Thread safety */
    pthread_mutex_lock(&g_malloc_mutex);
    
    /* Get chunk from pointer */
    t_chunk *chunk = get_chunk_from_ptr(ptr);
    if (!chunk) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;
    }
    
    /* Validate chunk */
    if (!chunk_validate(chunk) || chunk->state != CHUNK_ALLOCATED) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Invalid chunk in realloc");
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;
    }
    
    size_t old_size = chunk->size;
    size_t aligned_new_size = ALIGN_UP(size);
    
    /* Check if current size is sufficient */
    if (old_size >= aligned_new_size) {
        /* Can shrink or keep same size */
        if (old_size > aligned_new_size + MIN_CHUNK_SIZE) {
            /* Split chunk to return excess memory */
            chunk_split(chunk, aligned_new_size);
        }
        pthread_mutex_unlock(&g_malloc_mutex);
        return ptr;
    }
    
    /* Try to expand in place */
    if (try_expand_in_place(chunk, aligned_new_size) == 0) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return ptr;
    }
    
    /* Cannot expand in place - need new allocation */
    pthread_mutex_unlock(&g_malloc_mutex);
    
    return copy_data_to_new_allocation(ptr, old_size, size);
}
