/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_malloc.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 11:58:45 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_common.h"

/* Static test result for this module */
static test_result_t g_test_result = {0, 0, NULL};
#define TEST_END() printf("Passed: %d, Failed: %d\n", g_test_result.passed, g_test_result.failed)

/* Test functions */

/**
 * Test basic malloc functionality
 */
int test_malloc_basic(void)
{
    TEST_START("Basic Malloc Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test 1: Basic allocation
    void *ptr1 = malloc(100);
    TEST_ASSERT(ptr1 != NULL, "Basic malloc(100) should succeed");
    
    // Test 2: Write to allocated memory
    if (ptr1) {
        memset(ptr1, 0x42, 100);
        TEST_ASSERT(((char*)ptr1)[0] == 0x42, "Memory should be writable");
        TEST_ASSERT(((char*)ptr1)[99] == 0x42, "Memory should be writable at end");
        free(ptr1);
    }
    
    // Test 3: Zero size allocation
    void *ptr2 = malloc(0);
    TEST_ASSERT(ptr2 == NULL, "malloc(0) should return NULL");
    
    // Test 4: Very small allocation
    void *ptr3 = malloc(1);
    TEST_ASSERT(ptr3 != NULL, "malloc(1) should succeed");
    if (ptr3) {
        *(char*)ptr3 = 'A';
        TEST_ASSERT(*(char*)ptr3 == 'A', "Single byte allocation should work");
        free(ptr3);
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test different allocation sizes
 */
int test_malloc_sizes(void)
{
    TEST_START("Malloc Size Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test tiny allocations (< 256 bytes)
    void *tiny = malloc(100);
    TEST_ASSERT(tiny != NULL, "Tiny allocation (100 bytes) should succeed");
    
    // Test small allocations (256 bytes - 1KB)
    void *small = malloc(512);
    TEST_ASSERT(small != NULL, "Small allocation (512 bytes) should succeed");
    
    // Test medium allocations (1KB - 4KB)
    void *medium = malloc(2048);
    TEST_ASSERT(medium != NULL, "Medium allocation (2KB) should succeed");
    
    // Test large allocations (> 4KB)
    void *large = malloc(8192);
    TEST_ASSERT(large != NULL, "Large allocation (8KB) should succeed");
    
    // Verify they're all different
    TEST_ASSERT(tiny != small && small != medium && medium != large, 
                "All allocations should have different addresses");
    
    // Test alignment (should be 8-byte aligned on most systems)
    TEST_ASSERT(((uintptr_t)tiny % 8) == 0, "Tiny allocation should be aligned");
    TEST_ASSERT(((uintptr_t)small % 8) == 0, "Small allocation should be aligned");
    TEST_ASSERT(((uintptr_t)medium % 8) == 0, "Medium allocation should be aligned");
    TEST_ASSERT(((uintptr_t)large % 8) == 0, "Large allocation should be aligned");
    
    // Clean up
    if (tiny) free(tiny);
    if (small) free(small);
    if (medium) free(medium);
    if (large) free(large);
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test multiple allocations
 */
int test_malloc_multiple(void)
{
    TEST_START("Multiple Malloc Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    #define NUM_ALLOCS 10
    void *ptrs[NUM_ALLOCS];
    
    // Allocate multiple blocks
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(64 + i * 32);  // Variable sizes
        TEST_ASSERT(ptrs[i] != NULL, "Multiple allocation should succeed");
        
        // Write unique pattern to each block
        if (ptrs[i]) {
            memset(ptrs[i], i + 0x30, 64 + i * 32);
        }
    }
    
    // Verify all blocks are unique
    for (int i = 0; i < NUM_ALLOCS; i++) {
        for (int j = i + 1; j < NUM_ALLOCS; j++) {
            TEST_ASSERT(ptrs[i] != ptrs[j], "Each allocation should be unique");
        }
    }
    
    // Verify data integrity
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if (ptrs[i]) {
            TEST_ASSERT(((char*)ptrs[i])[0] == (i + 0x30), 
                       "Data integrity should be maintained");
        }
    }
    
    // Clean up
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test edge cases and error conditions
 */
int test_malloc_edge_cases(void)
{
    TEST_START("Malloc Edge Case Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test very large allocation (should fail gracefully)
    void *huge = malloc(SIZE_MAX / 2);  // Half of SIZE_MAX to avoid compiler warning
    TEST_ASSERT(huge == NULL, "Huge allocation should fail gracefully");
    
    // Test maximum reasonable size
    void *large = malloc(1024 * 1024);  // 1MB
    TEST_ASSERT(large != NULL, "1MB allocation should succeed");
    if (large) free(large);
    
    // Test allocation after many small allocations
    void *small_ptrs[100];
    int successful_allocs = 0;
    
    for (int i = 0; i < 100; i++) {
        small_ptrs[i] = malloc(32);
        if (small_ptrs[i]) successful_allocs++;
    }
    
    TEST_ASSERT(successful_allocs > 50, "Should be able to allocate many small blocks");
    
    // Clean up small allocations
    for (int i = 0; i < 100; i++) {
        if (small_ptrs[i]) free(small_ptrs[i]);
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Run all malloc tests
 */
int run_malloc_tests(void)
{
    printf("\n==========================================\n");
    printf("         MALLOC TESTS\n");
    printf("==========================================\n");
    
    int total_score = 0;
    
    total_score += test_malloc_basic();
    total_score += test_malloc_sizes();
    total_score += test_malloc_multiple();
    total_score += test_malloc_edge_cases();
    
    printf("\n--- MALLOC TESTS SUMMARY ---\n");
    printf("Total Passed: %d\n", g_test_result.passed);
    printf("Total Failed: %d\n", g_test_result.failed);
    printf("Success Rate: %.1f%%\n", 
           g_test_result.passed + g_test_result.failed > 0 ? 
           (100.0 * g_test_result.passed) / (g_test_result.passed + g_test_result.failed) : 0.0);
    
    return total_score;
}

/* Main function for standalone testing */
#ifdef TEST_MALLOC_STANDALONE
int main(void)
{
    return run_malloc_tests() >= 0 ? 0 : 1;
}
#endif
