#include "../../include/malloc_internal.h"
#include <stdio.h>

void show_alloc_mem(void)
{
    pthread_mutex_lock(&g_mutex);

    size_t total = 0;
    const char *zone_names[] = {"TINY", "SMALL", "LARGE"};

    for (int type = 0; type < 3; type++) {
        t_zone *zone = g_manager.zones[type];
        int zone_iter = 0;

        while (zone && zone_iter < MAX_ZONES_PER_TYPE) {
            printf("%s : %p\n", zone_names[type], zone->start);

            t_chunk *chunk = zone->chunks;
            int chunk_iter = 0;
            while (chunk && chunk_iter < MAX_CHUNKS_PER_ZONE) {
                if (!chunk->is_free) {
                    void *user_ptr = get_user_ptr(chunk);
                    printf("%p - %p : %zu bytes\n",
                           user_ptr,
                           (char *)user_ptr + chunk->size,
                           chunk->size);
                    total += chunk->size;
                }
                chunk = chunk->next;
                chunk_iter++;
            }

            zone = zone->next;
            zone_iter++;
        }
    }

    printf("Total : %zu bytes\n", total);

    pthread_mutex_unlock(&g_mutex);
}
