/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   show_alloc_mem.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/19 10:00:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 10:22:01 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/malloc.h"
#include "../internal/metadata.h"
#include "../internal/zone_manager.h"
#include "../internal/malloc_constants.h"
#include <stdio.h>
#include <pthread.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Bounded operations with iteration limits
** - Thread-safe access to global state
** - Required output format compliance
*/

/* ===== External Declarations ===== */
extern pthread_mutex_t g_malloc_mutex;

/* ===== Helper Functions ===== */

/*
** Display zones of specific type
** NASA: Bounded iteration with zone validation
*/
static size_t show_zones_of_type(zone_type_t type, const char *type_name)
{
    t_zone_manager *manager = get_global_zone_manager();
    if (!manager) {
        return 0;
    }
    
    size_t total_allocated = 0;
    t_zone *zone = manager->zones[type];
    
    printf("%s : ", type_name);
    if (!zone) {
        printf("0x0\n");
        return 0;
    }
    
    /* Display first zone address */
    printf("%p\n", zone->start_addr);
    
    /* Iterate through all zones of this type */
    while (zone && total_allocated < MAX_ALLOC_SIZE) {
        if (!zone_validate(zone)) {
            break;
        }
        
        /* Iterate through chunks in this zone */
        t_chunk *chunk = zone->first_chunk;
        int chunk_count = 0;
        
        while (chunk && chunk_count < MAX_ITERATIONS) {
            if (!chunk_validate(chunk)) {
                break;
            }
            
            /* Only display allocated chunks */
            if (chunk->state == CHUNK_ALLOCATED) {
                void *start_addr = (char *)chunk + CHUNK_HEADER_SIZE;
                void *end_addr = (char *)start_addr + chunk->size;
                
                printf("%p - %p : %u bytes\n", 
                       start_addr, end_addr, chunk->size);
                
                total_allocated += chunk->size;
            }
            
            chunk = chunk->next;
            chunk_count++;
        }
        
        zone = zone->next;
    }
    
    return total_allocated;
}

/*
** Display memory allocation summary
** NASA: Comprehensive system state display
*/
void show_alloc_mem(void)
{
    /* Thread safety */
    pthread_mutex_lock(&g_malloc_mutex);
    
    size_t total_bytes = 0;
    
    /* Display TINY allocations */
    size_t tiny_bytes = show_zones_of_type(ZONE_TINY, "TINY");
    total_bytes += tiny_bytes;
    
    /* Display SMALL allocations */
    size_t small_bytes = show_zones_of_type(ZONE_SMALL, "SMALL");
    total_bytes += small_bytes;
    
    /* Display LARGE allocations */
    size_t large_bytes = show_zones_of_type(ZONE_LARGE, "LARGE");
    total_bytes += large_bytes;
    
    /* Display total */
    printf("Total : %zu bytes\n", total_bytes);
    
    pthread_mutex_unlock(&g_malloc_mutex);
}
