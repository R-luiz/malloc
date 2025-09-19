/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   zone_manager.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 10:25:02 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../internal/metadata.h"
#include "../wrappers/mmap_wrapper.h"
#include "../wrappers/error_handler.h"
#include "../internal/malloc_constants.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

/* External function declarations */
extern uint64_t get_timestamp_us(void);
extern uint64_t calculate_checksum(const void *data, size_t size);

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - No recursion
** - Bounded loops with MAX_ITERATIONS
** - Assert invariants and pre/post conditions
*/

/* ===== Global Zone Manager ===== */
static t_zone_manager g_zone_manager = {0};
static int g_manager_initialized = 0;

/* ===== Helper Functions ===== */

/*
** Calculate zone manager checksum
** NASA: Data integrity protection
*/
static uint64_t calculate_manager_checksum(const t_zone_manager *manager)
{
    const uint8_t *data = (const uint8_t *)manager;
    size_t size = sizeof(*manager) - sizeof(manager->stats_checksum);
    uint64_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        checksum = checksum * 31 + data[i];
    }
    return checksum;
}

/*
** Get zone size for type
** NASA: Centralized zone size policy
*/
static size_t get_zone_size_for_type(zone_type_t type)
{
    switch (type) {
        case ZONE_TINY:
            return TINY_ZONE_SIZE;
        case ZONE_SMALL:
            return SMALL_ZONE_SIZE;
        case ZONE_LARGE:
            return 0; /* Large allocations are direct mmap */
        default:
            return 0;
    }
}

/*
** Initialize zone header with validation
** NASA: Explicit initialization with bounds checking
*/
static int initialize_zone_header(t_zone *zone, zone_type_t type, 
                                 void *addr, size_t size)
{
    assert(zone != NULL && "NULL zone parameter");
    assert(addr != NULL && "NULL address parameter");
    assert(size > 0 && "Invalid size parameter");
    
    memset(zone, 0, sizeof(*zone));
    
    zone->magic = MAGIC_ZONE;
    zone->type = type;
    zone->total_size = (uint32_t)size;
    zone->used_size = 0;
    zone->start_addr = addr;
    zone->end_addr = (char *)addr + size;
    zone->first_chunk = NULL;
    zone->free_list = NULL;
    zone->next = NULL;
    zone->prev = NULL;
    zone->chunk_count = 0;
    zone->free_count = 0;
    zone->owner_pid = getpid();
    zone->creation_time = get_timestamp_us();
    zone->checksum = calculate_checksum(zone, sizeof(*zone) - sizeof(zone->checksum));
    
    return 0;
}

/* ===== Zone Management Functions ===== */

/*
** Create a new memory zone
** NASA: Bounded operation with comprehensive validation
*/
t_zone *zone_create(zone_type_t type, size_t requested_size)
{
    assert(g_manager_initialized && "Zone manager not initialized");
    assert(type < ZONE_TYPE_COUNT && "Invalid zone type");
    
    /* Validate zone type */
    if (type >= ZONE_TYPE_COUNT) {
        handle_error(ERROR_INVALID_ZONE_TYPE, "Invalid zone type");
        return NULL;
    }
    
    /* Determine zone size */
    size_t zone_size;
    if (type == ZONE_LARGE) {
        /* Large allocations are direct mmap with requested size */
        zone_size = align_to_page_size(requested_size + ZONE_HEADER_SIZE);
    } else {
        zone_size = get_zone_size_for_type(type);
    }
    
    if (zone_size == 0) {
        handle_error(ERROR_INVALID_SIZE, "Invalid zone size");
        return NULL;
    }
    
    /* Map memory for zone */
    void *zone_memory = safe_mmap(zone_size);
    if (!zone_memory) {
        handle_error(ERROR_ZONE_CREATION, "Failed to map zone memory");
        return NULL;
    }
    
    /* Zone header is at the beginning of mapped memory */
    t_zone *zone = (t_zone *)zone_memory;
    
    /* Initialize zone header */
    void *data_start = (char *)zone_memory + ZONE_HEADER_SIZE;
    size_t data_size = zone_size - ZONE_HEADER_SIZE;
    
    if (initialize_zone_header(zone, type, data_start, data_size) != 0) {
        safe_munmap(zone_memory, zone_size);
        handle_error(ERROR_ZONE_CREATION, "Failed to initialize zone header");
        return NULL;
    }
    
    /* For non-large zones, create initial free chunk */
    if (type != ZONE_LARGE) {
        t_chunk *initial_chunk = chunk_create(data_start, data_size - CHUNK_HEADER_SIZE, zone);
        if (!initial_chunk) {
            safe_munmap(zone_memory, zone_size);
            handle_error(ERROR_ZONE_CREATION, "Failed to create initial chunk");
            return NULL;
        }
        
        zone->first_chunk = initial_chunk;
        zone->free_list = initial_chunk;
        zone->chunk_count = 1;
        zone->free_count = 1;
    }
    
    /* Update zone checksum after initialization */
    zone->checksum = calculate_checksum(zone, sizeof(*zone) - sizeof(zone->checksum));
    
    return zone;
}

/*
** Validate zone integrity
** NASA: Runtime validation with comprehensive checks
*/
int zone_validate(const t_zone *zone)
{
    if (!zone) {
        return 0;
    }
    
    /* Check magic number */
    if (zone->magic != MAGIC_ZONE) {
        return 0;
    }
    
    /* Validate zone type */
    if (zone->type >= ZONE_TYPE_COUNT) {
        return 0;
    }
    
    /* Check address boundaries */
    if (!zone->start_addr || !zone->end_addr) {
        return 0;
    }
    
    if (zone->start_addr >= zone->end_addr) {
        return 0;
    }
    
    /* Validate size constraints */
    if (zone->used_size > zone->total_size) {
        return 0;
    }
    
    /* Verify checksum */
    uint64_t calculated_checksum = calculate_checksum(zone, sizeof(*zone) - sizeof(zone->checksum));
    if (calculated_checksum != zone->checksum) {
        return 0;
    }
    
    return 1;
}

/*
** Find free chunk in zone that can satisfy size requirement
** NASA: Bounded search with iteration limit
*/
t_chunk *zone_find_free_chunk(t_zone *zone, size_t size)
{
    assert(zone != NULL && "NULL zone parameter");
    assert(size > 0 && "Invalid size parameter");
    
    if (!zone_validate(zone)) {
        handle_error(ERROR_ZONE_CORRUPTION, "Zone validation failed");
        return NULL;
    }
    
    t_chunk *current = zone->free_list;
    int iterations = 0;
    
    /* Bounded search through free list */
    while (current && iterations < MAX_ITERATIONS) {
        if (chunk_validate(current) && 
            current->state == CHUNK_FREE &&
            current->size >= size) {
            return current;
        }
        
        current = current->next;
        iterations++;
    }
    
    if (iterations >= MAX_ITERATIONS) {
        handle_error(ERROR_INTERNAL_STATE, "Free chunk search exceeded iteration limit");
    }
    
    return NULL; /* No suitable chunk found */
}

/*
** Add chunk to zone's free list
** NASA: Bounded operation with validation
*/
int zone_add_chunk(t_zone *zone, t_chunk *chunk)
{
    assert(zone != NULL && "NULL zone parameter");
    assert(chunk != NULL && "NULL chunk parameter");
    
    if (!zone_validate(zone)) {
        handle_error(ERROR_ZONE_CORRUPTION, "Zone validation failed");
        return -1;
    }
    
    if (!chunk_validate(chunk)) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Chunk validation failed");
        return -1;
    }
    
    /* Set chunk zone reference */
    chunk->zone = zone;
    
    /* Add to chunk list */
    if (zone->first_chunk) {
        zone->first_chunk->prev = chunk;
    }
    chunk->next = zone->first_chunk;
    chunk->prev = NULL;
    zone->first_chunk = chunk;
    
    /* Add to free list if chunk is free */
    if (chunk->state == CHUNK_FREE) {
        chunk->next = zone->free_list;
        if (zone->free_list) {
            zone->free_list->prev = chunk;
        }
        zone->free_list = chunk;
        zone->free_count++;
    }
    
    zone->chunk_count++;
    
    /* Update zone checksum */
    zone->checksum = calculate_checksum(zone, sizeof(*zone) - sizeof(zone->checksum));
    
    return 0;
}

/*
** Coalesce adjacent free chunks in zone
** NASA: Bounded operation to reduce fragmentation
*/
int zone_coalesce_free_chunks(t_zone *zone)
{
    assert(zone != NULL && "NULL zone parameter");
    
    if (!zone_validate(zone)) {
        handle_error(ERROR_ZONE_CORRUPTION, "Zone validation failed");
        return -1;
    }
    
    t_chunk *current = zone->first_chunk;
    int iterations = 0;
    int coalesced_count = 0;
    
    while (current && iterations < MAX_ITERATIONS) {
        if (current->state == CHUNK_FREE && current->next &&
            current->next->state == CHUNK_FREE) {
            
            /* Check if chunks are adjacent */
            void *current_end = (char *)current + CHUNK_HEADER_SIZE + current->size;
            void *next_start = (char *)current->next;
            
            if (current_end == next_start) {
                /* Coalesce chunks */
                t_chunk *next_chunk = current->next;
                current->size += CHUNK_HEADER_SIZE + next_chunk->size;
                current->next = next_chunk->next;
                
                if (next_chunk->next) {
                    next_chunk->next->prev = current;
                }
                
                /* Remove next_chunk from free list */
                if (zone->free_list == next_chunk) {
                    zone->free_list = next_chunk->next;
                }
                
                zone->chunk_count--;
                zone->free_count--;
                coalesced_count++;
                
                /* Continue with current chunk to check for more coalescing */
                continue;
            }
        }
        
        current = current->next;
        iterations++;
    }
    
    if (iterations >= MAX_ITERATIONS) {
        handle_error(ERROR_INTERNAL_STATE, "Coalescing exceeded iteration limit");
        return -1;
    }
    
    /* Update zone checksum */
    zone->checksum = calculate_checksum(zone, sizeof(*zone) - sizeof(zone->checksum));
    
    return coalesced_count;
}

/* ===== Zone Manager Functions ===== */

/*
** Initialize zone manager
** NASA: Explicit initialization with validation
*/
int manager_init(t_zone_manager *manager)
{
    if (!manager) {
        return -1;
    }
    
    memset(manager, 0, sizeof(*manager));
    
    manager->magic = MAGIC_ZONE;
    manager->initialized = 1;
    manager->page_size = get_system_page_size();
    
    if (manager->page_size == 0) {
        handle_error(ERROR_INVALID_PAGE_SIZE, "Failed to get system page size");
        return -1;
    }
    
    /* Initialize zone lists */
    for (int i = 0; i < ZONE_TYPE_COUNT; i++) {
        manager->zones[i] = NULL;
        manager->zone_counts[i] = 0;
    }
    
    manager->stats_checksum = calculate_manager_checksum(manager);
    
    return 0;
}

/*
** Global zone manager initialization
** NASA: One-time system initialization
*/
int zone_manager_global_init(void)
{
    if (g_manager_initialized) {
        return 0; /* Already initialized */
    }
    
    if (manager_init(&g_zone_manager) != 0) {
        return -1;
    }
    
    g_manager_initialized = 1;
    return 0;
}

/*
** Get global zone manager
** NASA: Controlled access to global state
*/
t_zone_manager *get_global_zone_manager(void)
{
    if (!g_manager_initialized) {
        if (zone_manager_global_init() != 0) {
            return NULL;
        }
    }
    
    return &g_zone_manager;
}

/*
** Add zone to manager
** NASA: Bounded operation with validation
*/
int manager_add_zone(t_zone_manager *manager, t_zone *zone)
{
    assert(manager != NULL && "NULL manager parameter");
    assert(zone != NULL && "NULL zone parameter");
    
    if (!manager_validate(manager)) {
        handle_error(ERROR_INTERNAL_STATE, "Manager validation failed");
        return -1;
    }
    
    if (!zone_validate(zone)) {
        handle_error(ERROR_ZONE_CORRUPTION, "Zone validation failed");
        return -1;
    }
    
    zone_type_t type = zone->type;
    if (type >= ZONE_TYPE_COUNT) {
        handle_error(ERROR_INVALID_ZONE_TYPE, "Invalid zone type");
        return -1;
    }
    
    /* Check zone count limit */
    if (manager->zone_counts[type] >= MAX_ZONES_PER_TYPE) {
        handle_error(ERROR_MAX_ZONES_EXCEEDED, "Maximum zones exceeded");
        return -1;
    }
    
    /* Add zone to list */
    zone->next = manager->zones[type];
    if (manager->zones[type]) {
        manager->zones[type]->prev = zone;
    }
    zone->prev = NULL;
    manager->zones[type] = zone;
    manager->zone_counts[type]++;
    
    /* Update manager checksum */
    manager->stats_checksum = calculate_manager_checksum(manager);
    
    return 0;
}

/*
** Find zone that can satisfy allocation size
** NASA: Bounded search with iteration limit
*/
t_zone *manager_find_zone_for_size(t_zone_manager *manager, size_t size)
{
    assert(manager != NULL && "NULL manager parameter");
    assert(size > 0 && "Invalid size parameter");
    
    if (!manager_validate(manager)) {
        handle_error(ERROR_INTERNAL_STATE, "Manager validation failed");
        return NULL;
    }
    
    zone_type_t type = get_zone_type_for_size(size);
    t_zone *current = manager->zones[type];
    int iterations = 0;
    
    while (current && iterations < MAX_ITERATIONS) {
        if (zone_validate(current)) {
            t_chunk *free_chunk = zone_find_free_chunk(current, size);
            if (free_chunk) {
                return current;
            }
        }
        
        current = current->next;
        iterations++;
    }
    
    if (iterations >= MAX_ITERATIONS) {
        handle_error(ERROR_INTERNAL_STATE, "Zone search exceeded iteration limit");
    }
    
    return NULL; /* No suitable zone found */
}

/*
** Validate zone manager integrity
** NASA: Runtime validation with comprehensive checks
*/
int manager_validate(const t_zone_manager *manager)
{
    if (!manager) {
        return 0;
    }
    
    if (manager->magic != MAGIC_ZONE || !manager->initialized) {
        return 0;
    }
    
    if (manager->page_size == 0) {
        return 0;
    }
    
    /* Verify checksum */
    uint64_t calculated_checksum = calculate_manager_checksum(manager);
    if (calculated_checksum != manager->stats_checksum) {
        return 0;
    }
    
    return 1;
}
