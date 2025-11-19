#define _GNU_SOURCE
#include "../../include/malloc_internal.h"
#include <sys/mman.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#else
#define MAP_ANONYMOUS 0x20
#endif
#endif

t_zone_type get_zone_type(size_t size)
{
    if (size <= TINY_MAX)
        return ZONE_TINY;
    if (size <= SMALL_MAX)
        return ZONE_SMALL;
    return ZONE_LARGE;
}

size_t get_zone_size(t_zone_type type)
{
    if (type == ZONE_TINY)
        return TINY_ZONE_SIZE;
    if (type == ZONE_SMALL)
        return SMALL_ZONE_SIZE;
    return 0;
}

t_zone *create_zone(t_zone_type type, size_t min_size)
{
    if (type != ZONE_LARGE) {
        if (g_manager.zone_counts[type] >= MAX_ZONES_PER_TYPE)
            return NULL;
    }

    size_t zone_size = get_zone_size(type);

    if (type == ZONE_LARGE) {
        size_t page_size = GET_PAGE_SIZE();
        zone_size = ALIGN(min_size + ZONE_HEADER_SIZE + CHUNK_HEADER_SIZE);
        zone_size = ((zone_size + page_size - 1) / page_size) * page_size;
    }

    void *ptr = mmap(NULL, zone_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
        return NULL;

    t_zone *zone = (t_zone *)ptr;
    zone->type = type;
    zone->total_size = zone_size;
    zone->used_size = ZONE_HEADER_SIZE;
    zone->start = ptr;
    zone->end = (char *)ptr + zone_size;
    zone->chunks = NULL;
    zone->next = NULL;
    zone->chunk_count = 0;

    if (type != ZONE_LARGE)
        g_manager.zone_counts[type]++;

    return zone;
}

void add_zone_to_manager(t_zone *zone)
{
    t_zone_type type = zone->type;

    if (!g_manager.zones[type]) {
        g_manager.zones[type] = zone;
    } else {
        t_zone *current = g_manager.zones[type];
        int iterations = 0;
        while (current->next && iterations < MAX_ZONES_PER_TYPE) {
            current = current->next;
            iterations++;
        }
        if (iterations < MAX_ZONES_PER_TYPE)
            current->next = zone;
    }
}

t_zone *find_or_create_zone(t_zone_type type, size_t size)
{
    t_zone *zone = g_manager.zones[type];

    if (type == ZONE_LARGE) {
        zone = create_zone(type, size);
        if (zone)
            add_zone_to_manager(zone);
        return zone;
    }

    int iterations = 0;
    while (zone && iterations < MAX_ZONE_SEARCH) {
        t_chunk *chunk = find_free_chunk(zone, size);
        if (chunk)
            return zone;

        if (zone->used_size + CHUNK_HEADER_SIZE + size <= zone->total_size)
            return zone;

        zone = zone->next;
        iterations++;
    }

    zone = create_zone(type, size);
    if (zone)
        add_zone_to_manager(zone);

    return zone;
}

t_zone *find_zone_for_chunk(t_chunk *chunk)
{
    for (int type = 0; type < 3; type++) {
        t_zone *zone = g_manager.zones[type];
        int iterations = 0;
        while (zone && iterations < MAX_ZONES_PER_TYPE) {
            if ((void *)chunk >= zone->start && (void *)chunk < zone->end)
                return zone;
            zone = zone->next;
            iterations++;
        }
    }
    return NULL;
}
