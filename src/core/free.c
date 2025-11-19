#include "../../include/malloc_internal.h"
#include <sys/mman.h>

void free(void *ptr)
{
    if (!ptr)
        return;

    pthread_mutex_lock(&g_mutex);

    t_chunk *chunk = get_chunk_from_ptr(ptr);

    if (!validate_chunk(chunk)) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    if (chunk->is_free) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    t_zone *zone = chunk->zone;

    chunk->magic = CHUNK_MAGIC_FREE;
    chunk->is_free = 1;

    merge_adjacent_chunks(chunk, zone);

    if (zone && zone->type == ZONE_LARGE && is_zone_empty(zone)) {
        remove_zone_from_manager(zone);
        munmap(zone->start, zone->total_size);
    }

    pthread_mutex_unlock(&g_mutex);
}
