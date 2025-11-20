#include "../../include/malloc_internal.h"
#include <stdint.h>

static int validate_realloc_ptr(void *ptr)
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

    return 1;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (!validate_realloc_ptr(ptr))
        return NULL;

    t_chunk *chunk = get_chunk_from_ptr(ptr);
    size_t aligned_size = ALIGN(size);

    if (chunk->size >= aligned_size) {
        pthread_mutex_lock(&g_mutex);
        t_zone *zone = chunk->zone;
        split_chunk(chunk, aligned_size, zone);
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    size_t copy_size = (chunk->size < size) ? chunk->size : size;
    ft_memcpy(new_ptr, ptr, copy_size);
    free(ptr);

    return new_ptr;
}
