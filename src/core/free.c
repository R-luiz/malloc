#include "../../include/malloc_internal.h"
#include <sys/mman.h>
#include <stdint.h>

static int validate_free_ptr(void *ptr, t_chunk **out_chunk)
{
    if ((uintptr_t)ptr % ALIGNMENT != 0)
        return 0;

    t_chunk *chunk = get_chunk_from_ptr(ptr);
    if (!chunk)
        return 0;

    if (chunk->magic != CHUNK_MAGIC_ALLOCATED)
        return 0;

    if (chunk->is_free)
        return 0;

    if (!chunk->zone)
        return 0;

    if (!validate_zone(chunk->zone))
        return 0;

    if ((void *)chunk < chunk->zone->start ||
        (void *)chunk >= chunk->zone->end)
        return 0;

    void *expected_user_ptr = (char *)chunk + CHUNK_HEADER_SIZE;
    if (ptr != expected_user_ptr)
        return 0;

    *out_chunk = chunk;
    return 1;
}

void free(void *ptr)
{
    t_chunk *chunk;

    if (!ptr)
        return;

    pthread_mutex_lock(&g_mutex);

    if (!validate_free_ptr(ptr, &chunk)) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    t_zone *zone = chunk->zone;

    chunk->magic = CHUNK_MAGIC_FREE;
    chunk->is_free = 1;

    merge_adjacent_chunks(chunk, zone);

    if (zone->type == ZONE_LARGE && is_zone_empty(zone)) {
        remove_zone_from_manager(zone);
        munmap(zone->start, zone->total_size);
    }

    pthread_mutex_unlock(&g_mutex);
}
