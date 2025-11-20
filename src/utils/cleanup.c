#include "../../include/malloc_internal.h"
#include <sys/mman.h>

static int cleanup_empty_zones_of_type(t_zone_type type)
{
	t_zone *zone = g_manager.zones[type];
	t_zone *prev = NULL;
	int freed_count = 0;
	int zone_iter = 0;

	while (zone && zone_iter < MAX_ZONES_PER_TYPE) {
		t_zone *next_zone = zone->next;

		if (is_zone_empty(zone)) {
			if (prev)
				prev->next = next_zone;
			else
				g_manager.zones[type] = next_zone;

			munmap(zone->start, zone->total_size);

			if (type != ZONE_LARGE && g_manager.zone_counts[type] > 0)
				g_manager.zone_counts[type]--;

			freed_count++;
		} else {
			prev = zone;
		}

		zone = next_zone;
		zone_iter++;
	}

	return freed_count;
}

int malloc_cleanup(void)
{
	int total_freed = 0;

	pthread_mutex_lock(&g_mutex);

	for (int type = 0; type < 3; type++)
		total_freed += cleanup_empty_zones_of_type(type);

	pthread_mutex_unlock(&g_mutex);

	return total_freed;
}

static void secure_zero_zone(t_zone *zone)
{
	size_t size = zone->total_size;
	unsigned char *ptr = (unsigned char *)zone->start;

	ft_memset(ptr, 0, size);
}

static void destroy_all_zones_of_type(t_zone_type type)
{
	t_zone *zone = g_manager.zones[type];
	int zone_iter = 0;

	while (zone && zone_iter < MAX_ZONES_PER_TYPE) {
		t_zone *next_zone = zone->next;

		secure_zero_zone(zone);
		munmap(zone->start, zone->total_size);

		zone = next_zone;
		zone_iter++;
	}

	g_manager.zones[type] = NULL;
	if (type != ZONE_LARGE)
		g_manager.zone_counts[type] = 0;
}

void malloc_destroy(void)
{
	pthread_mutex_lock(&g_mutex);

	for (int type = 0; type < 3; type++)
		destroy_all_zones_of_type(type);

	pthread_mutex_unlock(&g_mutex);
}
