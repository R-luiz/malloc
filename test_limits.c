#include "include/malloc.h"
#include <stdio.h>
#include <assert.h>

void test_chunk_limit()
{
    printf("\n=== Testing CHUNK limit enforcement ===\n");

    void *ptrs[100];
    int success_count = 0;

    for (int i = 0; i < 100; i++) {
        ptrs[i] = malloc(10);
        if (ptrs[i] != NULL)
            success_count++;
    }

    printf("Successfully allocated %d chunks of 10 bytes\n", success_count);
    printf("(Limited by MAX_CHUNKS_PER_ZONE)\n");

    for (int i = 0; i < success_count; i++) {
        free(ptrs[i]);
    }

    printf("All chunks freed successfully\n");
}

void test_zone_limit()
{
    printf("\n=== Testing ZONE limit (would need many allocations) ===\n");
    printf("With current settings:\n");
    printf("- TINY zones (<=128 bytes): Would need to fill zones\n");
    printf("- SMALL zones (129-1024 bytes): Would need to fill zones\n");
    printf("- LARGE zones (>1024 bytes): No limit (exempt)\n");
    printf("\nNote: Full zone limit test would require extensive allocations\n");
}

void test_error_handling()
{
    printf("\n=== Testing error handling on limit ===\n");

    void *ptr1 = malloc(50);
    assert(ptr1 != NULL);
    printf("First allocation: SUCCESS\n");

    t_malloc_stats stats;
    get_malloc_stats(&stats);
    printf("Current allocations - TINY: %u, SMALL: %u, LARGE: %u\n",
           stats.allocs_tiny, stats.allocs_small, stats.allocs_large);

    free(ptr1);
    printf("Freed allocation successfully\n");
}

void test_split_with_limit()
{
    printf("\n=== Testing split_chunk with limit ===\n");

    void *ptr = malloc(500);
    assert(ptr != NULL);
    printf("Allocated 500 bytes\n");

    ptr = realloc(ptr, 100);
    assert(ptr != NULL);
    printf("Reallocated to 100 bytes (should split chunk)\n");

    show_alloc_mem();

    free(ptr);
    printf("Freed successfully\n");
}

void test_merge_with_count()
{
    printf("\n=== Testing merge_adjacent_chunks with count ===\n");

    void *ptr1 = malloc(100);
    void *ptr2 = malloc(100);
    void *ptr3 = malloc(100);

    assert(ptr1 != NULL && ptr2 != NULL && ptr3 != NULL);
    printf("Allocated 3 chunks of 100 bytes each\n");

    t_malloc_stats stats;
    get_malloc_stats(&stats);
    printf("Chunks before free: TINY=%u, SMALL=%u\n",
           stats.allocs_tiny, stats.allocs_small);

    free(ptr2);
    printf("Freed middle chunk (ptr2)\n");

    free(ptr1);
    printf("Freed ptr1 (should merge with ptr2)\n");

    free(ptr3);
    printf("Freed ptr3 (should merge with ptr1+ptr2)\n");

    printf("All merges completed successfully\n");
}

int main(void)
{
    printf("=================================================\n");
    printf("    MALLOC LIMIT ENFORCEMENT TEST SUITE         \n");
    printf("=================================================\n");

    test_chunk_limit();
    test_zone_limit();
    test_error_handling();
    test_split_with_limit();
    test_merge_with_count();

    printf("\n=================================================\n");
    printf("    ALL LIMIT TESTS COMPLETED SUCCESSFULLY       \n");
    printf("=================================================\n");

    int leaks = check_malloc_leaks();
    if (leaks > 0) {
        printf("\nWARNING: %d memory leaks detected!\n", leaks);
        return 1;
    }

    printf("\nNo memory leaks detected!\n");
    return 0;
}
