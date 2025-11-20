#include "../../include/malloc_internal.h"

int malloc_validate_system(void)
{
    return 0;
}

int get_malloc_stats(t_malloc_stats *stats)
{
    if (!stats)
        return -1;

    ft_memset(stats, 0, sizeof(t_malloc_stats));

    pthread_mutex_lock(&g_mutex);

    for (int type = 0; type < 3; type++) {
        t_zone *zone = g_manager.zones[type];
        int zone_iter = 0;
        while (zone && zone_iter < MAX_ZONES_PER_TYPE) {
            t_chunk *chunk = zone->chunks;
            int chunk_iter = 0;
            while (chunk && chunk_iter < MAX_CHUNKS_PER_ZONE) {
                if (!chunk->is_free) {
                    stats->bytes_allocated += chunk->size;
                    if (type == ZONE_TINY)
                        stats->allocs_tiny++;
                    else if (type == ZONE_SMALL)
                        stats->allocs_small++;
                    else
                        stats->allocs_large++;
                }
                chunk = chunk->next;
                chunk_iter++;
            }
            zone = zone->next;
            zone_iter++;
        }
    }

    pthread_mutex_unlock(&g_mutex);
    return 0;
}

int check_malloc_leaks(void)
{
    int leaks = 0;

    pthread_mutex_lock(&g_mutex);

    for (int type = 0; type < 3; type++) {
        t_zone *zone = g_manager.zones[type];
        int zone_iter = 0;
        while (zone && zone_iter < MAX_ZONES_PER_TYPE) {
            t_chunk *chunk = zone->chunks;
            int chunk_iter = 0;
            while (chunk && chunk_iter < MAX_CHUNKS_PER_ZONE) {
                if (!chunk->is_free)
                    leaks++;
                chunk = chunk->next;
                chunk_iter++;
            }
            zone = zone->next;
            zone_iter++;
        }
    }

    pthread_mutex_unlock(&g_mutex);
    return leaks;
}
