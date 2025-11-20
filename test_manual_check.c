#include <stdio.h>
#include "include/malloc_internal.h"

// Direct access to internals for debugging

void manual_leak_check(void) {
    printf("\n=== MANUAL LEAK CHECK ===\n");
    
    for (int type = 0; type < 3; type++) {
        const char *names[] = {"TINY", "SMALL", "LARGE"};
        printf("\n%s zones:\n", names[type]);
        
        t_zone *zone = g_manager.zones[type];
        int zone_count = 0;
        
        while (zone && zone_count < 5) {
            printf("  Zone %d: %zu chunks\n", zone_count, zone->chunk_count);
            
            t_chunk *chunk = zone->chunks;
            int chunk_count = 0;
            int allocated_count = 0;
            
            while (chunk && chunk_count < 100) {
                if (!chunk->is_free) {
                    allocated_count++;
                    printf("    LEAK: Chunk at %p, size=%zu, is_free=%d\n", 
                           (void*)chunk, chunk->size, chunk->is_free);
                }
                chunk = chunk->next;
                chunk_count++;
            }
            
            printf("  Zone %d has %d leaks\n", zone_count, allocated_count);
            zone = zone->next;
            zone_count++;
        }
    }
}

int main(void) {
    printf("=== Initial state (BEFORE first malloc) ===\n");
    manual_leak_check();
    
    printf("\n\n=== Allocating 100 bytes ===\n");
    void *ptr = malloc(100);
    manual_leak_check();
    
    printf("\n\n=== Freeing ===\n");
    free(ptr);
    manual_leak_check();
    
    return 0;
}
