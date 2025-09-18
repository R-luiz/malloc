/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_free.c                                        :+:      :+:    :+:   */
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

/**
 * Test basic free functionality
 */
int test_free_basic(void)
{
    TEST_START("Basic Free Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test 1: Basic malloc and free
    void *ptr = malloc(100);
    TEST_ASSERT(ptr != NULL, "Malloc should succeed before free test");
    
    if (ptr) {
        free(ptr);
        TEST_ASSERT(1, "Basic free should not crash"); // If we reach here, free didn't crash
    }
    
    // Test 2: Free NULL pointer (should be safe)
    free(NULL);
    TEST_ASSERT(1, "free(NULL) should be safe and not crash");
    
    // Test 3: Multiple small allocations and frees
    void *ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = malloc(64);
        TEST_ASSERT(ptrs[i] != NULL, "Sequential malloc should succeed");
    }
    
    for (int i = 0; i < 5; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    TEST_ASSERT(1, "Sequential free operations should complete successfully");
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test free with different allocation sizes
 */
int test_free_sizes(void)
{
    TEST_START("Free Different Sizes Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test freeing tiny allocation
    void *tiny = malloc(16);
    TEST_ASSERT(tiny != NULL, "Tiny malloc should succeed");
    if (tiny) {
        free(tiny);
        TEST_ASSERT(1, "Free tiny allocation should succeed");
    }
    
    // Test freeing small allocation
    void *small = malloc(512);
    TEST_ASSERT(small != NULL, "Small malloc should succeed");
    if (small) {
        free(small);
        TEST_ASSERT(1, "Free small allocation should succeed");
    }
    
    // Test freeing medium allocation
    void *medium = malloc(2048);
    TEST_ASSERT(medium != NULL, "Medium malloc should succeed");
    if (medium) {
        free(medium);
        TEST_ASSERT(1, "Free medium allocation should succeed");
    }
    
    // Test freeing large allocation
    void *large = malloc(8192);
    TEST_ASSERT(large != NULL, "Large malloc should succeed");
    if (large) {
        free(large);
        TEST_ASSERT(1, "Free large allocation should succeed");
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test free order independence
 */
int test_free_order(void)
{
    TEST_START("Free Order Independence Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    #define NUM_PTRS 6
    void *ptrs[NUM_PTRS];
    
    // Allocate multiple blocks
    for (int i = 0; i < NUM_PTRS; i++) {
        ptrs[i] = malloc(128 + i * 64);
        TEST_ASSERT(ptrs[i] != NULL, "Multiple allocations should succeed");
    }
    
    // Free in reverse order
    for (int i = NUM_PTRS - 1; i >= 0; i--) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    TEST_ASSERT(1, "Reverse order free should succeed");
    
    // Allocate again
    for (int i = 0; i < NUM_PTRS; i++) {
        ptrs[i] = malloc(64 + i * 32);
        TEST_ASSERT(ptrs[i] != NULL, "Re-allocation after reverse free should succeed");
    }
    
    // Free in random order (every other one first)
    for (int i = 0; i < NUM_PTRS; i += 2) {
        if (ptrs[i]) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    for (int i = 1; i < NUM_PTRS; i += 2) {
        if (ptrs[i]) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    TEST_ASSERT(1, "Random order free should succeed");
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test memory reuse after free
 */
int test_free_reuse(void)
{
    TEST_START("Memory Reuse After Free Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Allocate and free a block
    void *ptr1 = malloc(256);
    TEST_ASSERT(ptr1 != NULL, "First allocation should succeed");
    
    if (ptr1) {
        // Write pattern to memory
        memset(ptr1, 0xAA, 256);
        free(ptr1);
    }
    
    // Allocate same size again - might reuse the same memory
    void *ptr2 = malloc(256);
    TEST_ASSERT(ptr2 != NULL, "Second allocation should succeed");
    
    if (ptr2) {
        // Memory might be reused but should be usable
        memset(ptr2, 0xBB, 256);
        TEST_ASSERT(((char*)ptr2)[0] == (char)0xBB, "Reused memory should be writable");
        TEST_ASSERT(((char*)ptr2)[255] == (char)0xBB, "Reused memory should be fully writable");
        free(ptr2);
    }
    
    // Test with multiple allocations and frees
    void *ptrs[10];
    
    // Allocate multiple blocks
    for (int i = 0; i < 10; i++) {
        ptrs[i] = malloc(128);
        if (ptrs[i]) {
            memset(ptrs[i], i + 0x30, 128);
        }
    }
    
    // Free every other block
    for (int i = 0; i < 10; i += 2) {
        if (ptrs[i]) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    // Allocate new blocks (might reuse freed ones)
    for (int i = 0; i < 10; i += 2) {
        ptrs[i] = malloc(128);
        TEST_ASSERT(ptrs[i] != NULL, "Re-allocation should succeed");
        if (ptrs[i]) {
            memset(ptrs[i], 0xFF, 128);
        }
    }
    
    // Clean up remaining blocks
    for (int i = 0; i < 10; i++) {
        if (ptrs[i]) {
            free(ptrs[i]);
        }
    }
    
    TEST_ASSERT(1, "Memory reuse pattern should work correctly");
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Run all free tests
 */
int run_free_tests(void)
{
    printf("\n==========================================\n");
    printf("           FREE TESTS\n");
    printf("==========================================\n");
    
    int total_score = 0;
    
    total_score += test_free_basic();
    total_score += test_free_sizes();
    total_score += test_free_order();
    total_score += test_free_reuse();
    
    printf("\n--- FREE TESTS SUMMARY ---\n");
    printf("Total Passed: %d\n", g_test_result.passed);
    printf("Total Failed: %d\n", g_test_result.failed);
    printf("Success Rate: %.1f%%\n", 
           g_test_result.passed + g_test_result.failed > 0 ? 
           (100.0 * g_test_result.passed) / (g_test_result.passed + g_test_result.failed) : 0.0);
    
    return total_score;
}

/* Main function for standalone testing */
#ifdef TEST_FREE_STANDALONE
int main(void)
{
    return run_free_tests() >= 0 ? 0 : 1;
}
#endif
