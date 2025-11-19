#include "include/malloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_tiny_allocations()
{
    printf("\n=== Testing TINY allocations (<=128 bytes) ===\n");

    char *s1 = malloc(10);
    char *s2 = malloc(50);
    char *s3 = malloc(100);

    assert(s1 != NULL);
    assert(s2 != NULL);
    assert(s3 != NULL);

    strcpy(s1, "hello");
    strcpy(s2, "world");
    strcpy(s3, "tiny zone test");

    printf("s1: %s\n", s1);
    printf("s2: %s\n", s2);
    printf("s3: %s\n", s3);

    show_alloc_mem();
}

void test_small_allocations()
{
    printf("\n=== Testing SMALL allocations (129-1024 bytes) ===\n");

    char *s1 = malloc(200);
    char *s2 = malloc(500);
    char *s3 = malloc(1000);

    assert(s1 != NULL);
    assert(s2 != NULL);
    assert(s3 != NULL);

    memset(s1, 'A', 200);
    memset(s2, 'B', 500);
    memset(s3, 'C', 1000);

    printf("Allocated 200, 500, 1000 bytes\n");

    show_alloc_mem();
}

void test_large_allocations()
{
    printf("\n=== Testing LARGE allocations (>1024 bytes) ===\n");

    char *s1 = malloc(2048);
    char *s2 = malloc(4096);
    char *s3 = malloc(8192);

    assert(s1 != NULL);
    assert(s2 != NULL);
    assert(s3 != NULL);

    printf("Allocated 2048, 4096, 8192 bytes\n");

    show_alloc_mem();
}

void test_mixed_allocations()
{
    printf("\n=== Testing MIXED allocations ===\n");

    void *tiny = malloc(50);
    void *small = malloc(500);
    void *large = malloc(5000);

    assert(tiny != NULL);
    assert(small != NULL);
    assert(large != NULL);

    printf("Allocated: tiny(50), small(500), large(5000)\n");

    show_alloc_mem();
}

void test_free_and_reuse()
{
    printf("\n=== Testing FREE and zone reuse ===\n");

    char *s1 = malloc(100);
    char *s2 = malloc(100);
    char *s3 = malloc(100);

    strcpy(s1, "first");
    strcpy(s2, "second");
    strcpy(s3, "third");

    printf("Before free:\n");
    show_alloc_mem();

    free(s2);

    printf("\nAfter freeing s2:\n");
    show_alloc_mem();

    char *s4 = malloc(100);
    strcpy(s4, "reused");

    printf("\nAfter allocating s4 (should reuse freed space):\n");
    show_alloc_mem();
}

void test_realloc()
{
    printf("\n=== Testing REALLOC ===\n");

    char *s = malloc(50);
    strcpy(s, "original");
    printf("Original: %s\n", s);

    s = realloc(s, 200);
    printf("After realloc to 200: %s\n", s);

    s = realloc(s, 30);
    printf("After realloc to 30: %s\n", s);

    show_alloc_mem();
}

void test_stats()
{
    printf("\n=== Testing STATS ===\n");

    malloc(50);
    malloc(200);
    malloc(2000);

    t_malloc_stats stats;
    get_malloc_stats(&stats);

    printf("TINY allocations: %u\n", stats.allocs_tiny);
    printf("SMALL allocations: %u\n", stats.allocs_small);
    printf("LARGE allocations: %u\n", stats.allocs_large);
    printf("Total bytes: %zu\n", stats.bytes_allocated);
}

void test_zone_efficiency()
{
    printf("\n=== Testing zone efficiency (multiple allocations in same zone) ===\n");

    printf("Allocating 100 tiny chunks...\n");
    for (int i = 0; i < 100; i++) {
        void *p = malloc(64);
        assert(p != NULL);
    }

    show_alloc_mem();
}

int main(void)
{
    printf("=================================================\n");
    printf("         MALLOC IMPLEMENTATION TEST SUITE        \n");
    printf("=================================================\n");

    test_tiny_allocations();
    test_small_allocations();
    test_large_allocations();
    test_mixed_allocations();
    test_free_and_reuse();
    test_realloc();
    test_stats();
    test_zone_efficiency();

    printf("\n=================================================\n");
    printf("         ALL TESTS COMPLETED SUCCESSFULLY         \n");
    printf("=================================================\n");

    return 0;
}
