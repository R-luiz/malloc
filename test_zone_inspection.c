#include "include/malloc.h"
#include "include/malloc_internal.h"
#include <stdio.h>

// This is a diagnostic tool to inspect the internal state
void inspect_zones(void)
{
    printf("\n=== INSPECTING ALL ZONES ===\n");
    
    for (int type = 0; type < 3; type++) {
        const char *type_names[] = {"TINY", "SMALL", "LARGE"};
        printf("\n%s zones:\n", type_names[type]);
        
        t_zone *zone = g_manager.zones[type];
        int zone_count = 0;
        
        while (zone && zone_count < 10) {
            printf("  Zone %d at %p:\n", zone_count, (void*)zone);
            printf("    magic: 0x%x (expected: 0x%x)\n", zone->magic, ZONE_MAGIC);
            printf("    type: %d\n", zone->type);
            printf("    total_size: %zu\n", zone->total_size);
            printf("    used_size: %zu\n", zone->used_size);
            printf("    start: %p\n", zone->start);
            printf("    end: %p\n", zone->end);
            printf("    chunk_count: %zu\n", zone->chunk_count);
            printf("    chunks: %p\n", (void*)zone->chunks);
            
            // Inspect chunks
            t_chunk *chunk = zone->chunks;
            int chunk_count = 0;
            while (chunk && chunk_count < 10) {
                printf("      Chunk %d at %p:\n", chunk_count, (void*)chunk);
                printf("        magic: 0x%x\n", chunk->magic);
                printf("        size: %zu\n", chunk->size);
                printf("        is_free: %d\n", chunk->is_free);
                printf("        zone: %p\n", (void*)chunk->zone);
                chunk = chunk->next;
                chunk_count++;
            }
            
            zone = zone->next;
            zone_count++;
        }
    }
    printf("\n=== END INSPECTION ===\n\n");
}

int main(void)
{
    printf("Initial state (before any malloc):\n");
    inspect_zones();
    printf("Leak count: %d\n", check_malloc_leaks());
    
    printf("\n\nAllocating one chunk...\n");
    void *ptr = malloc(100);
    inspect_zones();
    printf("Leak count: %d\n", check_malloc_leaks());
    
    printf("\n\nFreeing the chunk...\n");
    free(ptr);
    inspect_zones();
    printf("Leak count: %d\n", check_malloc_leaks());
    
    return 0;
}
