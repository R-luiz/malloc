/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   show_alloc_mem.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:27 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/malloc.h"
#include "../internal/metadata.h"
#include "../internal/zone_manager.h"
#include <stdio.h>
#include <pthread.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Bounded output
** - Thread-safe access
** - Clear formatting
*/

/* ===== External Declarations ===== */
extern pthread_mutex_t g_malloc_mutex;

/*
** Display memory allocation information
** NASA: Bounded output with thread safety
*/
void show_alloc_mem(void)
{
    if (pthread_mutex_lock(&g_malloc_mutex) != 0) {
        printf("Error: Cannot lock mutex for show_alloc_mem\n");
        return;
    }
    
    printf("MALLOC MEMORY MAP\n");
    printf("=================\n\n");
    
    t_zone_manager *manager = get_global_zone_manager();
    if (!manager || !manager_validate(manager)) {
        printf("No valid memory allocations found.\n");
        goto cleanup;
    }
    
    size_t total_allocated = 0;
    
    /* Display each zone type */
    const char *zone_names[] = {"TINY", "SMALL", "LARGE"};
    
    for (int type = 0; type < ZONE_TYPE_COUNT; type++) {
        t_zone *zone = manager->zones[type];
        
        if (!zone) {
            continue;
        }
        
        printf("%s ZONES:\n", zone_names[type]);
        printf("---------\n");
        
        while (zone) {
            if (zone_validate(zone)) {
                printf("Zone %p - %p : %u bytes\n",
                       zone->start_addr, zone->end_addr, zone->total_size);
                
                t_chunk *chunk = zone->first_chunk;
                while (chunk) {
                    if (chunk_validate(chunk) && chunk->state == CHUNK_ALLOCATED) {
                        void *data_ptr = chunk_get_data_ptr(chunk);
                        printf("  %p - %p : %u bytes\n",
                               data_ptr,
                               (char*)data_ptr + chunk->size,
                               chunk->size);
                        total_allocated += chunk->size;
                    }
                    chunk = chunk->next;
                }
            }
            zone = zone->next;
        }
        printf("\n");
    }
    
    printf("Total allocated: %zu bytes\n", total_allocated);

cleanup:
    pthread_mutex_unlock(&g_malloc_mutex);
}

