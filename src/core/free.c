#include "../../include/malloc_internal.h"

void free(void *ptr)
{
    if (!ptr)
        return;

    pthread_mutex_lock(&g_mutex);

    t_chunk *chunk = get_chunk_from_ptr(ptr);
    if (!chunk) {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    t_zone *zone = find_zone_for_chunk(chunk);

    chunk->is_free = 1;
    merge_adjacent_chunks(chunk, zone);

    pthread_mutex_unlock(&g_mutex);
}
