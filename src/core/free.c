#include "../../include/malloc.h"
#include "../internal/metadata.h"
#include "../internal/zone_manager.h"
#include "../internal/malloc_constants.h"
#include "../wrappers/mmap_wrapper.h"
#include "../wrappers/error_handler.h"
#include <pthread.h>
#include <assert.h>

extern pthread_mutex_t g_malloc_mutex;
extern t_malloc_stats g_malloc_stats;

static t_chunk *get_chunk_from_ptr(void *ptr)
{
    if (!ptr)
        return NULL;
    
    t_chunk *chunk = (t_chunk *)((char *)ptr - CHUNK_HEADER_SIZE);
    
    if (chunk->magic != MAGIC_ALLOC) {
        handle_error(ERROR_CHUNK_CORRUPTION, "Invalid chunk magic");
        return NULL;
    }
    
    return chunk;
}

static int free_large_allocation(t_chunk *chunk)
{
    t_zone *zone = chunk->zone;
    if (!zone || zone->type != ZONE_LARGE)
        return -1;
    
    g_malloc_stats.bytes_allocated -= chunk->size;
    g_malloc_stats.allocs_large--;
    
    return safe_munmap(zone->start_addr, zone->total_size);
}

static int free_chunk_in_zone(t_chunk *chunk)
{
    if (chunk_set_state(chunk, CHUNK_FREE) != 0)
        return -1;
    
    t_zone *zone = chunk->zone;
    zone_add_chunk(zone, chunk);
    zone_coalesce_free_chunks(zone);
    
    g_malloc_stats.bytes_allocated -= chunk->size;
    if (zone->type == ZONE_TINY)
        g_malloc_stats.allocs_tiny--;
    else
        g_malloc_stats.allocs_small--;
    
    return 0;
}

void free(void *ptr)
{
    if (!ptr)
        return;
    
    pthread_mutex_lock(&g_malloc_mutex);
    
    t_chunk *chunk = get_chunk_from_ptr(ptr);
    if (!chunk) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return;
    }
    
    if (!chunk_validate(chunk) || chunk->state != CHUNK_ALLOCATED) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return;
    }
    
    t_zone *zone = chunk->zone;
    
    if (zone && zone->type == ZONE_LARGE) {
        free_large_allocation(chunk);
    } else {
        free_chunk_in_zone(chunk);
    }
    
    pthread_mutex_unlock(&g_malloc_mutex);
}
