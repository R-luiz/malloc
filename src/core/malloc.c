#include "../../include/malloc_internal.h"

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    pthread_mutex_lock(&g_mutex);

    size_t aligned_size = ALIGN(size);
    t_zone_type type = get_zone_type(aligned_size);

    t_zone *zone = find_or_create_zone(type, aligned_size);
    if (!zone) {
        pthread_mutex_unlock(&g_mutex);
        return NULL;
    }

    t_chunk *chunk = find_free_chunk(zone, aligned_size);

    if (chunk) {
        chunk->is_free = 0;
        split_chunk(chunk, aligned_size, zone);
    } else {
        chunk = create_chunk_in_zone(zone, aligned_size);
        if (!chunk) {
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
    }

    pthread_mutex_unlock(&g_mutex);
    return get_user_ptr(chunk);
}
