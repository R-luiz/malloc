#include <stdio.h>
#include <pthread.h>
#include "include/malloc_internal.h"

void inspect_zones(const char *label) {
    printf("\n[%s]\n", label);
    for (int type = 0; type < 3; type++) {
        const char *names[] = {"TINY", "SMALL", "LARGE"};
        t_zone *zone = g_manager.zones[type];
        if (zone) {
            printf("  %s: zone exists, chunk_count=%zu\n", names[type], zone->chunk_count);
            t_chunk *chunk = zone->chunks;
            while (chunk) {
                printf("    Chunk: size=%zu, is_free=%d\n", chunk->size, chunk->is_free);
                chunk = chunk->next;
            }
        } else {
            printf("  %s: no zone\n", names[type]);
        }
    }
}

int main(void) {
    printf("=== Testing pthread_mutex_lock impact ===\n");
    
    inspect_zones("BEFORE any pthread operation");
    
    printf("\nCalling pthread_mutex_lock(&g_mutex)...\n");
    pthread_mutex_lock(&g_mutex);
    
    inspect_zones("AFTER pthread_mutex_lock");
    
    pthread_mutex_unlock(&g_mutex);
    
    inspect_zones("AFTER pthread_mutex_unlock");
    
    printf("\n=== Now doing a malloc ===\n");
    void *ptr = malloc(100);
    
    inspect_zones("AFTER malloc(100)");
    
    free(ptr);
    
    inspect_zones("AFTER free");
    
    return 0;
}
