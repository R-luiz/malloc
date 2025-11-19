#include "../../include/malloc_internal.h"

t_chunk *create_chunk_in_zone(t_zone *zone, size_t size)
{
    size_t needed = CHUNK_HEADER_SIZE + size;

    if (zone->used_size + needed > zone->total_size)
        return NULL;

    void *chunk_addr = (char *)zone->start + zone->used_size;
    t_chunk *chunk = (t_chunk *)chunk_addr;

    chunk->size = size;
    chunk->is_free = 0;
    chunk->next = zone->chunks;
    chunk->prev = NULL;

    if (zone->chunks)
        zone->chunks->prev = chunk;
    zone->chunks = chunk;

    zone->used_size += needed;

    return chunk;
}

t_chunk *find_free_chunk(t_zone *zone, size_t size)
{
    t_chunk *current = zone->chunks;
    int iterations = 0;

    while (current && iterations < 10000) {
        if (current->is_free && current->size >= size)
            return current;
        current = current->next;
        iterations++;
    }

    return NULL;
}

void split_chunk(t_chunk *chunk, size_t size)
{
    if (chunk->size < size + CHUNK_HEADER_SIZE + MIN_SPLIT_SIZE)
        return;

    void *new_chunk_addr = (char *)chunk + CHUNK_HEADER_SIZE + size;
    t_chunk *new_chunk = (t_chunk *)new_chunk_addr;

    new_chunk->size = chunk->size - size - CHUNK_HEADER_SIZE;
    new_chunk->is_free = 1;
    new_chunk->next = chunk->next;
    new_chunk->prev = chunk;

    if (chunk->next)
        chunk->next->prev = new_chunk;
    chunk->next = new_chunk;
    chunk->size = size;
}

void merge_adjacent_chunks(t_chunk *chunk)
{
    if (!chunk->is_free)
        return;

    if (chunk->next && chunk->next->is_free) {
        void *expected_next = (char *)chunk + CHUNK_HEADER_SIZE + chunk->size;
        if (expected_next == (void *)chunk->next) {
            chunk->size += CHUNK_HEADER_SIZE + chunk->next->size;
            chunk->next = chunk->next->next;
            if (chunk->next)
                chunk->next->prev = chunk;
        }
    }

    if (chunk->prev && chunk->prev->is_free) {
        void *expected_addr = (char *)chunk->prev + CHUNK_HEADER_SIZE + chunk->prev->size;
        if (expected_addr == (void *)chunk) {
            chunk->prev->size += CHUNK_HEADER_SIZE + chunk->size;
            chunk->prev->next = chunk->next;
            if (chunk->next)
                chunk->next->prev = chunk->prev;
        }
    }
}

void *get_user_ptr(t_chunk *chunk)
{
    return (char *)chunk + CHUNK_HEADER_SIZE;
}

t_chunk *get_chunk_from_ptr(void *ptr)
{
    if (!ptr)
        return NULL;
    return (t_chunk *)((char *)ptr - CHUNK_HEADER_SIZE);
}
