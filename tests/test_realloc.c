/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_realloc.c                                     :+:      :+:    :+:   */
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
 * Test basic realloc functionality
 */
int test_realloc_basic(void)
{
    TEST_START("Basic Realloc Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test 1: realloc with NULL pointer (should act like malloc)
    void *ptr1 = realloc(NULL, 100);
    TEST_ASSERT(ptr1 != NULL, "realloc(NULL, 100) should act like malloc");
    
    if (ptr1) {
        // Write test pattern
        memset(ptr1, 0x42, 100);
        TEST_ASSERT(((char*)ptr1)[0] == 0x42, "Memory should be writable");
        TEST_ASSERT(((char*)ptr1)[99] == 0x42, "Memory should be writable at end");
        free(ptr1);
    }
    
    // Test 2: realloc with size 0 (should act like free)
    void *ptr2 = malloc(50);
    TEST_ASSERT(ptr2 != NULL, "Initial malloc should succeed");
    
    if (ptr2) {
        void *result = realloc(ptr2, 0);
        TEST_ASSERT(result == NULL, "realloc(ptr, 0) should act like free and return NULL");
    }
    
    // Test 3: Basic resize (expand)
    void *ptr3 = malloc(50);
    TEST_ASSERT(ptr3 != NULL, "Initial malloc should succeed");
    
    if (ptr3) {
        // Write pattern to original memory
        memset(ptr3, 0xAA, 50);
        
        void *ptr3_new = realloc(ptr3, 100);
        TEST_ASSERT(ptr3_new != NULL, "realloc expansion should succeed");
        
        if (ptr3_new) {
            // Check that original data is preserved
            TEST_ASSERT(((char*)ptr3_new)[0] == (char)0xAA, "Original data should be preserved");
            TEST_ASSERT(((char*)ptr3_new)[49] == (char)0xAA, "Original data should be preserved at end");
            
            // Test that we can write to the new area
            memset((char*)ptr3_new + 50, 0xBB, 50);
            TEST_ASSERT(((char*)ptr3_new)[50] == (char)0xBB, "New area should be writable");
            TEST_ASSERT(((char*)ptr3_new)[99] == (char)0xBB, "New area should be writable at end");
            
            free(ptr3_new);
        }
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test realloc with size changes
 */
int test_realloc_sizes(void)
{
    TEST_START("Realloc Size Change Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test shrinking
    void *ptr = malloc(1000);
    TEST_ASSERT(ptr != NULL, "Initial large allocation should succeed");
    
    if (ptr) {
        // Fill with pattern
        for (int i = 0; i < 1000; i++) {
            ((char*)ptr)[i] = (char)(i % 256);
        }
        
        // Shrink to 500 bytes
        void *ptr_shrunk = realloc(ptr, 500);
        TEST_ASSERT(ptr_shrunk != NULL, "Shrinking realloc should succeed");
        
        if (ptr_shrunk) {
            // Check that data is preserved
            for (int i = 0; i < 500; i++) {
                if (((char*)ptr_shrunk)[i] != (char)(i % 256)) {
                    TEST_ASSERT(0, "Data should be preserved when shrinking");
                    break;
                }
            }
            if (g_test_result.current_test == NULL) // No failure detected
                TEST_ASSERT(1, "Data preserved correctly when shrinking");
            
            // Expand again
            void *ptr_expanded = realloc(ptr_shrunk, 1500);
            TEST_ASSERT(ptr_expanded != NULL, "Expanding realloc should succeed");
            
            if (ptr_expanded) {
                // Check that original data is still there
                for (int i = 0; i < 500; i++) {
                    if (((char*)ptr_expanded)[i] != (char)(i % 256)) {
                        TEST_ASSERT(0, "Data should be preserved when expanding");
                        break;
                    }
                }
                if (g_test_result.current_test == NULL) // No failure detected
                    TEST_ASSERT(1, "Data preserved correctly when expanding");
                
                // Test new area is writable
                memset((char*)ptr_expanded + 500, 0xFF, 1000);
                TEST_ASSERT(((char*)ptr_expanded)[500] == (char)0xFF, "New area should be writable");
                TEST_ASSERT(((char*)ptr_expanded)[1499] == (char)0xFF, "New area should be writable at end");
                
                free(ptr_expanded);
            }
        }
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test realloc with different allocation types
 */
int test_realloc_types(void)
{
    TEST_START("Realloc Different Types Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test tiny to small
    void *tiny = malloc(64);
    TEST_ASSERT(tiny != NULL, "Tiny malloc should succeed");
    
    if (tiny) {
        memset(tiny, 0x11, 64);
        void *small = realloc(tiny, 512);
        TEST_ASSERT(small != NULL, "Tiny to small realloc should succeed");
        
        if (small) {
            TEST_ASSERT(((char*)small)[0] == 0x11, "Data should be preserved in tiny to small");
            TEST_ASSERT(((char*)small)[63] == 0x11, "Data should be preserved in tiny to small");
            free(small);
        }
    }
    
    // Test small to large
    void *small2 = malloc(512);
    TEST_ASSERT(small2 != NULL, "Small malloc should succeed");
    
    if (small2) {
        memset(small2, 0x22, 512);
        void *large = realloc(small2, 8192);
        TEST_ASSERT(large != NULL, "Small to large realloc should succeed");
        
        if (large) {
            TEST_ASSERT(((char*)large)[0] == 0x22, "Data should be preserved in small to large");
            TEST_ASSERT(((char*)large)[511] == 0x22, "Data should be preserved in small to large");
            free(large);
        }
    }
    
    // Test large to small
    void *large2 = malloc(8192);
    TEST_ASSERT(large2 != NULL, "Large malloc should succeed");
    
    if (large2) {
        memset(large2, 0x33, 8192);
        void *small3 = realloc(large2, 256);
        TEST_ASSERT(small3 != NULL, "Large to small realloc should succeed");
        
        if (small3) {
            TEST_ASSERT(((char*)small3)[0] == 0x33, "Data should be preserved in large to small");
            TEST_ASSERT(((char*)small3)[255] == 0x33, "Data should be preserved in large to small");
            free(small3);
        }
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Test realloc edge cases
 */
int test_realloc_edge_cases(void)
{
    TEST_START("Realloc Edge Cases Tests");
    int initial_passed = g_test_result.passed;
    int initial_failed = g_test_result.failed;
    
    // Test realloc with same size
    void *ptr = malloc(100);
    TEST_ASSERT(ptr != NULL, "Initial malloc should succeed");
    
    if (ptr) {
        memset(ptr, 0x55, 100);
        void *same_size = realloc(ptr, 100);
        TEST_ASSERT(same_size != NULL, "Same size realloc should succeed");
        
        if (same_size) {
            TEST_ASSERT(((char*)same_size)[0] == 0x55, "Data should be preserved in same size realloc");
            TEST_ASSERT(((char*)same_size)[99] == 0x55, "Data should be preserved in same size realloc");
            free(same_size);
        }
    }
    
    // Test multiple reallocs
    void *multi = malloc(32);
    TEST_ASSERT(multi != NULL, "Initial allocation for multiple reallocs should succeed");
    
    if (multi) {
        // Set initial pattern
        memset(multi, 0x77, 32);
        
        // Chain of reallocs
        size_t sizes[] = {64, 128, 96, 256, 48, 512};
        size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
        
        for (size_t i = 0; i < num_sizes; i++) {
            void *new_multi = realloc(multi, sizes[i]);
            TEST_ASSERT(new_multi != NULL, "Chained realloc should succeed");
            
            if (new_multi) {
                // Check that at least the beginning is preserved
                size_t check_size = sizes[i] < 32 ? sizes[i] : 32;
                int preserved = 1;
                for (size_t j = 0; j < check_size; j++) {
                    if (((char*)new_multi)[j] != 0x77) {
                        preserved = 0;
                        break;
                    }
                }
                TEST_ASSERT(preserved, "Data should be preserved in chained reallocs");
                multi = new_multi;
            } else {
                break;
            }
        }
        
        if (multi) {
            free(multi);
        }
    }
    
    return (g_test_result.passed - initial_passed) - (g_test_result.failed - initial_failed);
}

/**
 * Run all realloc tests
 */
int run_realloc_tests(void)
{
    printf("\n==========================================\n");
    printf("         REALLOC TESTS\n");
    printf("==========================================\n");
    
    int total_score = 0;
    
    total_score += test_realloc_basic();
    total_score += test_realloc_sizes();
    total_score += test_realloc_types();
    total_score += test_realloc_edge_cases();
    
    printf("\n--- REALLOC TESTS SUMMARY ---\n");
    printf("Total Passed: %d\n", g_test_result.passed);
    printf("Total Failed: %d\n", g_test_result.failed);
    printf("Success Rate: %.1f%%\n", 
           g_test_result.passed + g_test_result.failed > 0 ? 
           (100.0 * g_test_result.passed) / (g_test_result.passed + g_test_result.failed) : 0.0);
    
    return total_score;
}

/* Main function for standalone testing */
#ifdef TEST_REALLOC_STANDALONE
int main(void)
{
    return run_realloc_tests() >= 0 ? 0 : 1;
}
#endif
