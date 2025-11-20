#include "../../include/malloc_internal.h"

static int zone_has_allocations(t_zone *zone)
{
    t_chunk *chunk = zone->chunks;
    int chunk_iter = 0;

    while (chunk && chunk_iter < MAX_CHUNKS_PER_ZONE) {
        if (!chunk->is_free)
            return 1;
        chunk = chunk->next;
        chunk_iter++;
    }
    return 0;
}

static void print_zone_allocations(t_zone *zone, size_t *total)
{
    t_chunk *chunk = zone->chunks;
    int chunk_iter = 0;

    while (chunk && chunk_iter < MAX_CHUNKS_PER_ZONE) {
        if (!chunk->is_free) {
            void *user_ptr = get_user_ptr(chunk);
            void *end_ptr = (char *)user_ptr + chunk->size;
            print_allocation(user_ptr, end_ptr, chunk->size);
            *total += chunk->size;
        }
        chunk = chunk->next;
        chunk_iter++;
    }
}

void show_alloc_mem(void)
{
    pthread_mutex_lock(&g_mutex);

    size_t total = 0;
    const char *zone_names[] = {"TINY", "SMALL", "LARGE"};

    for (int type = 0; type < 3; type++) {
        t_zone *zone = g_manager.zones[type];
        int zone_iter = 0;

        while (zone && zone_iter < MAX_ZONES_PER_TYPE) {
            if (zone_has_allocations(zone)) {
                print_zone_header(zone_names[type], zone->start);
                print_zone_allocations(zone, &total);
            }
            zone = zone->next;
            zone_iter++;
        }
    }

    print_total(total);

    pthread_mutex_unlock(&g_mutex);
}
