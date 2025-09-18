/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chunk_manager.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../internal/metadata.h"
#include "../wrappers/error_handler.h"
#include <string.h>
#include <assert.h>
#include <time.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - No recursion
** - Assert invariants and pre/post conditions
** - Bounded operations with iteration limits
*/

/* ===== Helper Functions ===== */

/*
** Calculate checksum for data integrity
** NASA: Data integrity protection with bounded operation
*/
uint64_t calculate_checksum(const void *data, size_t size)
{
    assert(data != NULL && "NULL data parameter");
    assert(size > 0 && "Invalid size parameter");
    
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t checksum = 0;
    size_t max_size = (size < 10000) ? size : 10000; /* Bound checksum calculation */
    
    for (size_t i = 0; i < max_size; i++) {
        checksum = checksum * 31 + bytes[i];
    }
    
    return checksum;
}

/*
** Get current timestamp in microseconds
** NASA: Bounded time calculation
*/
uint64_t get_timestamp_us(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0; /* Return 0 on error rather than undefined value */
    }
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

/*
** Verify memory alignment
** NASA: Alignment verification
*/
int verify_alignment(const void *ptr)
{
    if (!ptr) {
        return 0;
    }
    
    uintptr_t addr = (uintptr_t)ptr;
    return (addr % ALIGNMENT) == 0;
}

/*
** Get zone type for allocation size
** NASA: Size categorization policy
*/
zone_type_t get_zone_type_for_size(size_t size)
{
    if (size <= TINY_MAX) {
        return ZONE_TINY;
    } else if (size <= SMALL_MAX) {
        return ZONE_SMALL;
    } else {
        return ZONE_LARGE;
    }
}

/* ===== Chunk Management Functions ===== */

/*
** Create chunk with validation and initialization
** NASA: Comprehensive initialization with bounds checking
*/
t_chunk *chunk_create(void *addr, size_t size, t_zone *zone)
{
    assert(addr != NULL && "NULL address parameter");
    assert(size >= MIN_CHUNK_SIZE && "Size too small");
    assert(zone != NULL && "NULL zone parameter");
    
    /* Validate parameters */
    if (!addr || size < MIN_CHUNK_SIZE || !zone) {
        handle_error(ERROR_INVALID_PARAMETER, "Invalid chunk creation parameters");
        return NULL;
    }
    
    /* Verify alignment */
    if (!verify_alignment(addr)) {
        handle_error(ERROR_ALIGNMENT, "Address not properly aligned");
        return NULL;
    }
    
    /* Ensure we have enough space for chunk header */
    if (size < CHUNK_HEADER_SIZE + MIN_CHUNK_SIZE) {
        handle_error(ERROR_INVALID_SIZE, "Insufficient space for chunk header");
        return NULL;
    }
    
    /* Initialize chunk at the given address */
    t_chunk *chunk = (t_chunk *)addr;
    memset(chunk, 0, sizeof(*chunk));
    
    chunk->magic = MAGIC_FREE;  /* Start as free chunk */
    chunk->size = (uint32_t)(size - CHUNK_HEADER_SIZE);  /* Data size */
    chunk->state = CHUNK_FREE;
    chunk->next = NULL;
    chunk->prev = NULL;
    chunk->zone = zone;
    
    /* Calculate integrity checksum */
    chunk->checksum = calculate_checksum(chunk, sizeof(*chunk) - sizeof(chunk->checksum));
    
    return chunk;
}

/*
** Validate chunk integrity and structure
** NASA: Runtime validation with comprehensive checks
*/
int chunk_validate(const t_chunk *chunk)
{
    if (!chunk) {
        return 0;
    }
    
    /* Check magic number */
    if (chunk->magic != MAGIC_ALLOC && chunk->magic != MAGIC_FREE) {
        return 0;
    }
    
    /* Validate state */
    if (chunk->state != CHUNK_FREE && chunk->state != CHUNK_ALLOCATED && 
        chunk->state != CHUNK_CORRUPTED) {
        return 0;
    }
    
    /* Check size bounds */
    if (chunk->size == 0 || chunk->size > MAX_ALLOC_SIZE) {
        return 0;
    }
    
    /* Verify zone reference */
    if (!chunk->zone) {
        return 0;
    }
    
    /* Verify checksum */
    uint64_t calculated_checksum = calculate_checksum(chunk, sizeof(*chunk) - sizeof(chunk->checksum));
    if (calculated_checksum != chunk->checksum) {
        return 0;
    }
    
    return 1;
}

/*
** Set chunk state with validation and magic number update
** NASA: State management with integrity protection
*/
int chunk_set_state(t_chunk *chunk, chunk_state_t state)
{
    assert(chunk != NULL && "NULL chunk parameter");
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        return -1;
    }
    
    /* Validate state transition */
    if (state != CHUNK_FREE && state != CHUNK_ALLOCATED && state != CHUNK_CORRUPTED) {
        handle_error(ERROR_INVALID_CHUNK_STATE, "Invalid chunk state");
        return -1;
    }
    
    /* Prevent invalid state transitions */
    if (chunk->state == CHUNK_CORRUPTED && state != CHUNK_CORRUPTED) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Cannot change state of corrupted chunk");
        return -1;
    }
    
    /* Update state and magic number */
    chunk->state = state;
    chunk->magic = (state == CHUNK_FREE) ? MAGIC_FREE : MAGIC_ALLOC;
    
    /* Update checksum */
    chunk->checksum = calculate_checksum(chunk, sizeof(*chunk) - sizeof(chunk->checksum));
    
    return 0;
}

/*
** Get total chunk size including header
** NASA: Size calculation with validation
*/
size_t chunk_get_total_size(const t_chunk *chunk)
{
    assert(chunk != NULL && "NULL chunk parameter");
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        return 0;
    }
    
    return CHUNK_HEADER_SIZE + chunk->size;
}

/*
** Get user data pointer from chunk
** NASA: Safe pointer calculation with validation
*/
void *chunk_get_data_ptr(const t_chunk *chunk)
{
    assert(chunk != NULL && "NULL chunk parameter");
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        return NULL;
    }
    
    if (chunk->state != CHUNK_ALLOCATED) {
        handle_error(ERROR_INVALID_CHUNK_STATE, "Chunk not allocated");
        return NULL;
    }
    
    return (char *)chunk + CHUNK_HEADER_SIZE;
}

/*
** Get chunk from user data pointer
** NASA: Reverse pointer calculation with validation
*/
t_chunk *chunk_from_data_ptr(void *data_ptr)
{
    assert(data_ptr != NULL && "NULL data pointer");
    
    if (!verify_alignment(data_ptr)) {
        handle_error(ERROR_ALIGNMENT, "Data pointer not properly aligned");
        return NULL;
    }
    
    t_chunk *chunk = (t_chunk *)((char *)data_ptr - CHUNK_HEADER_SIZE);
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Invalid chunk from data pointer");
        return NULL;
    }
    
    return chunk;
}

/*
** Split chunk to create two chunks
** NASA: Controlled chunk splitting with bounds checking
*/
t_chunk *chunk_split(t_chunk *chunk, size_t size)
{
    assert(chunk != NULL && "NULL chunk parameter");
    assert(size > 0 && "Invalid size parameter");
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        return NULL;
    }
    
    if (chunk->state != CHUNK_FREE) {
        handle_error(ERROR_INVALID_CHUNK_STATE, "Cannot split non-free chunk");
        return NULL;
    }
    
    size_t aligned_size = ALIGN_UP(size);
    
    /* Check if split is possible and worthwhile */
    size_t min_remainder = CHUNK_HEADER_SIZE + MIN_CHUNK_SIZE;
    if (chunk->size < aligned_size + min_remainder) {
        /* Not enough space to split or not worth splitting */
        return chunk;
    }
    
    /* Calculate new chunk position */
    char *new_chunk_addr = (char *)chunk + CHUNK_HEADER_SIZE + aligned_size;
    size_t new_chunk_size = chunk->size - aligned_size;
    
    /* Create new chunk for remainder */
    t_chunk *new_chunk = chunk_create(new_chunk_addr, new_chunk_size, chunk->zone);
    if (!new_chunk) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Failed to create split chunk");
        return NULL;
    }
    
    /* Update original chunk size */
    chunk->size = (uint32_t)aligned_size;
    
    /* Link chunks */
    new_chunk->next = chunk->next;
    new_chunk->prev = chunk;
    if (chunk->next) {
        chunk->next->prev = new_chunk;
    }
    chunk->next = new_chunk;
    
    /* Update checksums */
    chunk->checksum = calculate_checksum(chunk, sizeof(*chunk) - sizeof(chunk->checksum));
    new_chunk->checksum = calculate_checksum(new_chunk, sizeof(*new_chunk) - sizeof(new_chunk->checksum));
    
    return chunk;
}

/*
** Merge chunk with next adjacent chunk
** NASA: Safe chunk merging with validation
*/
int chunk_merge_with_next(t_chunk *chunk)
{
    assert(chunk != NULL && "NULL chunk parameter");
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        return -1;
    }
    
    if (chunk->state != CHUNK_FREE) {
        handle_error(ERROR_INVALID_CHUNK_STATE, "Cannot merge non-free chunk");
        return -1;
    }
    
    t_chunk *next_chunk = chunk->next;
    if (!next_chunk || !chunk_validate(next_chunk)) {
        return 0; /* No next chunk or invalid */
    }
    
    if (next_chunk->state != CHUNK_FREE) {
        return 0; /* Next chunk not free */
    }
    
    /* Check if chunks are adjacent */
    char *chunk_end = (char *)chunk + CHUNK_HEADER_SIZE + chunk->size;
    char *next_start = (char *)next_chunk;
    
    if (chunk_end != next_start) {
        return 0; /* Chunks not adjacent */
    }
    
    /* Merge chunks */
    chunk->size += CHUNK_HEADER_SIZE + next_chunk->size;
    chunk->next = next_chunk->next;
    
    if (next_chunk->next) {
        next_chunk->next->prev = chunk;
    }
    
    /* Invalidate merged chunk */
    next_chunk->magic = MAGIC_CORRUPTED;
    
    /* Update checksum */
    chunk->checksum = calculate_checksum(chunk, sizeof(*chunk) - sizeof(chunk->checksum));
    
    return 1; /* Merge successful */
}

/*
** Check if two chunks are adjacent
** NASA: Adjacency checking for coalescing
*/
int chunks_are_adjacent(const t_chunk *chunk1, const t_chunk *chunk2)
{
    assert(chunk1 != NULL && "NULL chunk1 parameter");
    assert(chunk2 != NULL && "NULL chunk2 parameter");
    
    if (!chunk_validate(chunk1) || !chunk_validate(chunk2)) {
        return 0;
    }
    
    char *chunk1_end = (char *)chunk1 + CHUNK_HEADER_SIZE + chunk1->size;
    char *chunk2_start = (char *)chunk2;
    
    char *chunk2_end = (char *)chunk2 + CHUNK_HEADER_SIZE + chunk2->size;
    char *chunk1_start = (char *)chunk1;
    
    return (chunk1_end == chunk2_start) || (chunk2_end == chunk1_start);
}

/*
** Mark chunk as corrupted
** NASA: Corruption handling with state preservation
*/
void chunk_mark_corrupted(t_chunk *chunk)
{
    if (!chunk) {
        return;
    }
    
    chunk->magic = MAGIC_CORRUPTED;
    chunk->state = CHUNK_CORRUPTED;
    /* Don't update checksum for corrupted chunks */
}
