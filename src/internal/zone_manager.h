/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   zone_manager.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ZONE_MANAGER_H
# define ZONE_MANAGER_H

# include "metadata.h"
# include "malloc_constants.h"

/*
** NASA C Standard Compliance:
** - Zone-based memory management
** - TINY zones (1-128 bytes, 16KB zones)
** - SMALL zones (129-1024 bytes, 104KB zones)  
** - LARGE zones (>1024 bytes, direct mmap)
** - Free list management per zone
** - Coalesce adjacent free chunks
*/

/* ===== Zone Management Function Prototypes ===== */

/*
** Initialize global zone manager
** Returns: 0 on success, -1 on failure
** NASA: One-time system initialization
*/
int         zone_manager_global_init(void);

/*
** Get global zone manager instance
** Returns: Pointer to global manager or NULL on error
** NASA: Controlled access to global state
*/
t_zone_manager *get_global_zone_manager(void);

/*
** Create a new memory zone for given type and size
** Pre-conditions: type < ZONE_TYPE_COUNT, requested_size > 0
** Post-conditions: Returns valid zone or NULL
** NASA: Bounded operation with comprehensive validation
*/
t_zone      *zone_create(zone_type_t type, size_t requested_size);

/*
** Destroy a memory zone and free its resources
** Pre-conditions: zone != NULL, zone from zone_create
** Post-conditions: Zone memory unmapped, zone invalidated
** NASA: Explicit resource cleanup
*/
int         zone_destroy(t_zone *zone);

/*
** Validate zone integrity and structure
** Returns: 1 if valid, 0 if corrupted
** NASA: Runtime validation with comprehensive checks
*/
int         zone_validate(const t_zone *zone);

/*
** Find free chunk in zone that can satisfy size requirement
** Pre-conditions: zone != NULL, size > 0
** Post-conditions: Returns suitable chunk or NULL
** NASA: Bounded search with iteration limit
*/
t_chunk     *zone_find_free_chunk(t_zone *zone, size_t size);

/*
** Add chunk to zone's management structures
** Pre-conditions: zone != NULL, chunk != NULL
** Post-conditions: Chunk added to zone lists
** NASA: Bounded operation with validation
*/
int         zone_add_chunk(t_zone *zone, t_chunk *chunk);

/*
** Remove chunk from zone's management structures
** Pre-conditions: zone != NULL, chunk != NULL, chunk in zone
** Post-conditions: Chunk removed from zone lists
** NASA: Safe removal with validation
*/
int         zone_remove_chunk(t_zone *zone, t_chunk *chunk);

/*
** Coalesce adjacent free chunks in zone
** Pre-conditions: zone != NULL
** Post-conditions: Adjacent free chunks merged
** NASA: Bounded operation to reduce fragmentation
*/
int         zone_coalesce_free_chunks(t_zone *zone);

/*
** Split chunk to satisfy allocation request
** Pre-conditions: chunk != NULL, chunk->size >= size
** Post-conditions: Chunk split, remainder added to free list
** NASA: Controlled chunk splitting with bounds checking
*/
t_chunk     *zone_split_chunk(t_chunk *chunk, size_t size);

/*
** Add zone to manager's tracking structures
** Pre-conditions: manager != NULL, zone != NULL
** Post-conditions: Zone added to appropriate list
** NASA: Bounded operation with validation
*/
int         manager_add_zone(t_zone_manager *manager, t_zone *zone);

/*
** Remove zone from manager's tracking structures
** Pre-conditions: manager != NULL, zone != NULL, zone in manager
** Post-conditions: Zone removed from manager lists
** NASA: Safe removal with validation
*/
int         manager_remove_zone(t_zone_manager *manager, t_zone *zone);

/*
** Find zone that can satisfy allocation size
** Pre-conditions: manager != NULL, size > 0
** Post-conditions: Returns suitable zone or NULL
** NASA: Bounded search with iteration limit
*/
t_zone      *manager_find_zone_for_size(t_zone_manager *manager, size_t size);

/*
** Get zone type for given allocation size
** Pre-conditions: size > 0
** Post-conditions: Returns appropriate zone type
** NASA: Size categorization policy
*/
zone_type_t get_zone_type_for_size(size_t size);

/*
** Validate zone manager integrity
** Returns: 1 if valid, 0 if corrupted
** NASA: Runtime validation with comprehensive checks
*/
int         manager_validate(const t_zone_manager *manager);

/*
** Get zone statistics for monitoring
** Pre-conditions: zone != NULL, stats != NULL
** Post-conditions: stats filled with zone information
** NASA: Bounded data collection for monitoring
*/
int         zone_get_stats(const t_zone *zone, t_malloc_stats *stats);

/*
** Get manager statistics for monitoring
** Pre-conditions: manager != NULL, stats != NULL
** Post-conditions: stats filled with manager information
** NASA: Comprehensive system monitoring
*/
int         manager_get_stats(const t_zone_manager *manager, t_malloc_stats *stats);

/*
** Cleanup all zones and reset manager
** Pre-conditions: manager != NULL
** Post-conditions: All zones destroyed, manager reset
** NASA: Complete system cleanup
*/
int         manager_cleanup(t_zone_manager *manager);

/*
** Check for memory leaks in zone system
** Returns: 0 if no leaks, positive number of leaks detected
** NASA: Resource leak detection
*/
int         zone_check_leaks(void);

/*
** Print zone information for debugging
** Pre-conditions: zone != NULL
** NASA: Debugging support with bounded output
*/
void        zone_print_info(const t_zone *zone);

/*
** Print manager information for debugging
** Pre-conditions: manager != NULL
** NASA: System state debugging
*/
void        manager_print_info(const t_zone_manager *manager);

#endif

#ifndef ZONE_MANAGER_H
# define ZONE_MANAGER_H

/* Zone management internal functions and structures */

#endif
