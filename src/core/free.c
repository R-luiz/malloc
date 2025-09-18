/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:36 by rluiz             #+#    #+#             */
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
** - Double-free protection
** - Chunk validation and coalescing
*/

/* ===== External Declarations ===== */
extern pthread_mutex_t g_malloc_mutex;
extern t_malloc_stats g_malloc_stats;

/* External function declarations */
extern t_chunk *chunk_from_data_ptr(void *data_ptr);
extern int chunk_validate(const t_chunk *chunk);
extern int chunk_set_state(t_chunk *chunk, chunk_state_t state);
extern int chunk_merge_with_next(t_chunk *chunk);
extern size_t chunk_get_total_size(const t_chunk *chunk);

/* ===== Helper Functions ===== */

/*
** Validate free parameters
** NASA: Comprehensive parameter validation
*/
static int validate_free_params(void *ptr)
{
    if (!ptr) {
        /* free(NULL) is valid and should do nothing */
        return 0;
    }
    
    /* Verify pointer alignment */
    if (!verify_alignment(ptr)) {
        handle_error(ERROR_ALIGNMENT, "Pointer not properly aligned");
        return -1;
    }
    
    return 1; /* Valid non-NULL pointer */
}

/*
** Check for double-free condition
** NASA: Corruption detection and prevention
*/
static int check_double_free(const t_chunk *chunk)
{
    assert(chunk != NULL && "NULL chunk parameter");
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Invalid chunk in double-free check");
        return -1;
    }
    
    if (chunk->state == CHUNK_FREE) {
        handle_error(ERROR_DOUBLE_FREE, "Double free detected");
        return -1;
    }
    
    if (chunk->state == CHUNK_CORRUPTED) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Attempt to free corrupted chunk");
        return -1;
    }
    
    if (chunk->state != CHUNK_ALLOCATED) {
        handle_error(ERROR_INVALID_CHUNK_STATE, "Invalid chunk state for free");
        return -1;
    }
    
    return 0; /* No double-free detected */
}

/*
** Main free implementation
** NASA: Thread-safe deallocation with validation
*/
void free(void *ptr)
{
    /* Thread safety */
    if (pthread_mutex_lock(&g_malloc_mutex) != 0) {
        handle_error(ERROR_THREAD_SAFETY, "Failed to acquire free mutex");
        return;
    }
    
    /* Validate parameters */
    int param_validation = validate_free_params(ptr);
    if (param_validation == 0) {
        /* free(NULL) - valid operation, do nothing */
        goto cleanup;
    } else if (param_validation < 0) {
        /* Invalid parameter */
        goto cleanup;
    }
    
    /* Get chunk from user pointer */
    t_chunk *chunk = chunk_from_data_ptr(ptr);
    if (!chunk) {
        handle_error(ERROR_CHUNK_NOT_FOUND, "Cannot find chunk for pointer");
        goto cleanup;
    }
    
    /* Validate chunk integrity */
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        goto cleanup;
    }
    
    /* Check for double-free */
    if (check_double_free(chunk) != 0) {
        goto cleanup;
    }
    
    /* Mark chunk as free */
    if (chunk_set_state(chunk, CHUNK_FREE) != 0) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Failed to set chunk free state");
        goto cleanup;
    }

cleanup:
    /* Release mutex */
    if (pthread_mutex_unlock(&g_malloc_mutex) != 0) {
        handle_error(ERROR_THREAD_SAFETY, "Failed to release free mutex");
    }
}

