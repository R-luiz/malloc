#include "../../include/malloc_internal.h"
#include <string.h>

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    if (size == 0) {
        free(ptr);
        return NULL;
    }

    t_chunk *chunk = get_chunk_from_ptr(ptr);
    if (!chunk)
        return NULL;

    size_t aligned_size = ALIGN(size);

    if (chunk->size >= aligned_size) {
        pthread_mutex_lock(&g_mutex);
        split_chunk(chunk, aligned_size);
        pthread_mutex_unlock(&g_mutex);
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    size_t copy_size = (chunk->size < size) ? chunk->size : size;
    memcpy(new_ptr, ptr, copy_size);
    free(ptr);

    return new_ptr;
}
