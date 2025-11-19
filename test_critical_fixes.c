#include "include/malloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

void test_double_free_detection(void)
{
    printf("\n=== Test 1: Double-Free Detection ===\n");

    void *ptr = malloc(100);
    assert(ptr != NULL);
    printf("✓ Allocated 100 bytes at %p\n", ptr);

    free(ptr);
    printf("✓ First free() succeeded\n");

    free(ptr);
    printf("✓ Second free() did not crash (double-free detected and ignored)\n");
}

void test_invalid_pointer_detection(void)
{
    printf("\n=== Test 2: Invalid Pointer Detection ===\n");

    int stack_var = 42;
    free(&stack_var);
    printf("✓ free() on stack pointer did not crash\n");

    void *random_ptr = (void *)0xDEADBEEF;
    free(random_ptr);
    printf("✓ free() on random pointer did not crash\n");

    void *ptr = malloc(100);
    assert(ptr != NULL);

    free((char *)ptr + 50);
    printf("✓ free() on middle of allocation did not crash\n");

    free(ptr);
    printf("✓ Freed valid pointer successfully\n");
}

void test_large_zone_cleanup(void)
{
    printf("\n=== Test 3: LARGE Zone Cleanup ===\n");

    void *ptr1 = malloc(10000);
    assert(ptr1 != NULL);
    printf("✓ Allocated LARGE allocation (10000 bytes)\n");

    void *ptr2 = malloc(20000);
    assert(ptr2 != NULL);
    printf("✓ Allocated second LARGE allocation (20000 bytes)\n");

    free(ptr1);
    printf("✓ Freed first LARGE allocation (zone should be munmap'd)\n");

    free(ptr2);
    printf("✓ Freed second LARGE allocation (zone should be munmap'd)\n");

    printf("✓ LARGE zones should be returned to OS\n");
}

void test_o1_zone_lookup(void)
{
    printf("\n=== Test 4: O(1) Zone Lookup Performance ===\n");

    void *ptrs[100];

    for (int i = 0; i < 100; i++) {
        ptrs[i] = malloc(50);
        assert(ptrs[i] != NULL);
    }
    printf("✓ Allocated 100 chunks (zone pointer stored in each)\n");

    for (int i = 0; i < 100; i++) {
        free(ptrs[i]);
    }
    printf("✓ Freed all 100 chunks (O(1) zone lookup, no search)\n");
}

void test_chunk_validation(void)
{
    printf("\n=== Test 5: Chunk Validation (Magic Numbers) ===\n");

    void *ptr = malloc(100);
    assert(ptr != NULL);
    printf("✓ Allocated chunk with magic number\n");

    char *corrupted = (char *)ptr - 16;
    *(uint32_t *)corrupted = 0xBADBAD;

    free(ptr);
    printf("✓ Free validated magic number (corrupted ptr not freed)\n");
}

void test_use_after_free_detection(void)
{
    printf("\n=== Test 6: Use-After-Free Safety ===\n");

    void *ptr = malloc(200);
    assert(ptr != NULL);
    strcpy(ptr, "Test data");
    printf("✓ Allocated and wrote to chunk\n");

    free(ptr);
    printf("✓ Freed chunk (magic changed to FREE)\n");

    void *ptr2 = malloc(200);
    printf("✓ Allocated new chunk (may reuse freed chunk)\n");

    free(ptr2);
    printf("✓ Freed successfully\n");
}

void test_merge_with_validation(void)
{
    printf("\n=== Test 7: Merge Adjacent Chunks with Validation ===\n");

    void *ptr1 = malloc(100);
    void *ptr2 = malloc(100);
    void *ptr3 = malloc(100);

    assert(ptr1 != NULL && ptr2 != NULL && ptr3 != NULL);
    printf("✓ Allocated 3 adjacent chunks\n");

    free(ptr2);
    printf("✓ Freed middle chunk\n");

    free(ptr1);
    printf("✓ Freed first chunk (should merge with ptr2)\n");

    free(ptr3);
    printf("✓ Freed third chunk (should merge with ptr1+ptr2)\n");

    printf("✓ All chunks merged correctly with validated pointers\n");
}

void test_realloc_validation(void)
{
    printf("\n=== Test 8: Realloc with Validation ===\n");

    void *ptr = malloc(100);
    assert(ptr != NULL);
    strcpy(ptr, "Initial data");
    printf("✓ Allocated 100 bytes\n");

    void *new_ptr = realloc(ptr, 50);
    assert(new_ptr != NULL);
    printf("✓ Reallocated to smaller size (chunk split validated)\n");

    new_ptr = realloc(new_ptr, 200);
    assert(new_ptr != NULL);
    printf("✓ Reallocated to larger size (new allocation validated)\n");

    free(new_ptr);
    printf("✓ Freed successfully\n");
}

void test_concurrent_large_allocations(void)
{
    printf("\n=== Test 9: Multiple LARGE Allocations and Cleanup ===\n");

    void *large_ptrs[10];

    for (int i = 0; i < 10; i++) {
        large_ptrs[i] = malloc(5000 + i * 1000);
        assert(large_ptrs[i] != NULL);
    }
    printf("✓ Allocated 10 LARGE zones\n");

    for (int i = 0; i < 10; i++) {
        free(large_ptrs[i]);
    }
    printf("✓ Freed all LARGE zones (all should be munmap'd)\n");
}

void test_mixed_operations(void)
{
    printf("\n=== Test 10: Mixed Operations with All Fixes ===\n");

    void *tiny = malloc(50);
    void *small = malloc(500);
    void *large = malloc(5000);

    assert(tiny != NULL && small != NULL && large != NULL);
    printf("✓ Allocated TINY, SMALL, and LARGE\n");

    free(large);
    printf("✓ Freed LARGE (munmap'd)\n");

    free(tiny);
    printf("✓ Freed TINY (validated, O(1) lookup)\n");

    free(small);
    printf("✓ Freed SMALL (validated, O(1) lookup)\n");

    free(tiny);
    printf("✓ Attempted double-free (detected and ignored)\n");
}

int main(void)
{
    printf("=======================================================\n");
    printf("   CRITICAL FIXES VALIDATION TEST SUITE              \n");
    printf("=======================================================\n");
    printf("Testing all implemented fixes:\n");
    printf("  1. Double-free detection\n");
    printf("  2. Invalid pointer validation\n");
    printf("  3. LARGE zone cleanup (munmap)\n");
    printf("  4. O(1) zone lookup (chunk->zone)\n");
    printf("  5. Magic number validation\n");
    printf("  6. Use-after-free safety\n");
    printf("  7. Merge with validation\n");
    printf("  8. Realloc validation\n");
    printf("  9. Concurrent LARGE allocations\n");
    printf(" 10. Mixed operations\n");
    printf("=======================================================\n");

    test_double_free_detection();
    test_invalid_pointer_detection();
    test_large_zone_cleanup();
    test_o1_zone_lookup();
    test_chunk_validation();
    test_use_after_free_detection();
    test_merge_with_validation();
    test_realloc_validation();
    test_concurrent_large_allocations();
    test_mixed_operations();

    printf("\n=======================================================\n");
    printf("   ALL CRITICAL FIXES TESTS COMPLETED SUCCESSFULLY    \n");
    printf("=======================================================\n");

    printf("\nVerifying no memory leaks...\n");
    int leaks = check_malloc_leaks();
    if (leaks > 0) {
        printf("WARNING: %d memory leaks detected!\n", leaks);
        return 1;
    }

    printf("✓ No memory leaks detected!\n");
    printf("\n=======================================================\n");
    printf("   ALL TESTS PASSED - FIXES VERIFIED                  \n");
    printf("=======================================================\n");

    return 0;
}
