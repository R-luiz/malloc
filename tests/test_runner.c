/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_runner.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 12:01:23 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_common.h"

/* External test function declarations */
extern int run_malloc_tests(void);
extern int run_free_tests(void);
extern int run_realloc_tests(void);

/* Test suite structure */
typedef struct {
    const char *name;
    int (*test_func)(void);
    int score;
    int enabled;
} test_suite_t;

/* Global test results */
static int g_total_score = 0;

/**
 * Print test header
 */
void print_header(void)
{
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────┐\n");
    printf("│                    MALLOC TEST SUITE                       │\n");
    printf("│                                                            │\n");
    printf("│  Tests: malloc(), free(), realloc(), show_alloc_mem()      │\n");
    printf("│  Author: rluiz                                             │\n");
    printf("│  Date: %s                                        │\n", __DATE__);
    printf("└────────────────────────────────────────────────────────────┘\n");
    printf("\n");
}

/**
 * Print test summary
 */
void print_summary(test_suite_t *suites, int num_suites)
{
    printf("\n");
    printf("┌────────────────────────────────────────────────────────────┐\n");
    printf("│                      TEST SUMMARY                          │\n");
    printf("└────────────────────────────────────────────────────────────┘\n");
    
    printf("\nIndividual Test Suite Results:\n");
    printf("─────────────────────────────────\n");
    
    for (int i = 0; i < num_suites; i++) {
        if (suites[i].enabled) {
            const char *color = suites[i].score >= 0 ? "✓" : "✗";
            printf("%s %-15s: Score %3d\n", color, suites[i].name, suites[i].score);
        } else {
            printf("- %-15s: SKIPPED\n", suites[i].name);
        }
    }
    
    printf("\nOverall Results:\n");
    printf("────────────────\n");
    printf("Total Score: %d\n", g_total_score);
    
    if (g_total_score >= 0) {
        printf("🎉 ALL TESTS PASSED! Your malloc implementation is working correctly.\n");
    } else {
        printf("❌ Some tests failed. Please review the implementation.\n");
    }
    
    printf("\nMemory Status:\n");
    printf("──────────────\n");
    show_alloc_mem();
    printf("\n");
}

/**
 * Run individual test suite with timing
 */
int run_test_suite(test_suite_t *suite)
{
    if (!suite->enabled) {
        printf("⏭️  Skipping %s tests (disabled)\n", suite->name);
        return 0;
    }
    
    printf("🚀 Starting %s tests...\n", suite->name);
    
    clock_t start = clock();
    suite->score = suite->test_func();
    clock_t end = clock();
    
    double duration = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    const char *status = suite->score >= 0 ? "✅ PASSED" : "❌ FAILED";
    printf("📊 %s tests %s (Score: %d, Time: %.3fs)\n", 
           suite->name, status, suite->score, duration);
    
    g_total_score += suite->score;
    
    return suite->score;
}

/**
 * Test show_alloc_mem function
 */
int test_show_alloc_mem(void)
{
    printf("\n==========================================\n");
    printf("      SHOW_ALLOC_MEM TESTS\n");
    printf("==========================================\n");
    
    printf("\n=== Testing show_alloc_mem with no allocations ===\n");
    show_alloc_mem();
    
    printf("\n=== Testing show_alloc_mem with various allocations ===\n");
    void *tiny = malloc(64);
    void *small = malloc(512);
    void *large = malloc(8192);
    
    printf("After allocating tiny(64), small(512), large(8192):\n");
    show_alloc_mem();
    
    free(small);
    printf("\nAfter freeing small allocation:\n");
    show_alloc_mem();
    
    free(tiny);
    free(large);
    printf("\nAfter freeing all allocations:\n");
    show_alloc_mem();
    
    printf("\n--- SHOW_ALLOC_MEM TESTS SUMMARY ---\n");
    printf("show_alloc_mem() executed successfully\n");
    
    return 1; // Always pass if no crash
}

/**
 * Performance stress test
 */
int test_performance(void)
{
    printf("\n==========================================\n");
    printf("        PERFORMANCE TESTS\n");
    printf("==========================================\n");
    
    const int NUM_ALLOCS = 1000;
    void *ptrs[NUM_ALLOCS];
    
    // Test allocation speed
    clock_t start = clock();
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = malloc(64 + (i % 512));
    }
    clock_t end = clock();
    
    double alloc_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("✓ Allocated %d blocks in %.3f seconds\n", NUM_ALLOCS, alloc_time);
    
    // Test free speed
    start = clock();
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    end = clock();
    
    double free_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("✓ Freed %d blocks in %.3f seconds\n", NUM_ALLOCS, free_time);
    
    printf("\n--- PERFORMANCE TESTS SUMMARY ---\n");
    printf("Allocation rate: %.0f allocs/sec\n", NUM_ALLOCS / alloc_time);
    printf("Free rate: %.0f frees/sec\n", NUM_ALLOCS / free_time);
    
    return 1;
}

/**
 * Main test runner
 */
int main(int argc, char **argv)
{
    print_header();
    
    // Define test suites
    test_suite_t suites[] = {
        {"malloc", run_malloc_tests, 0, 1},
        {"free", run_free_tests, 0, 1},
        {"realloc", run_realloc_tests, 0, 1},
        {"show_mem", test_show_alloc_mem, 0, 1},
        {"performance", test_performance, 0, 1}
    };
    
    int num_suites = sizeof(suites) / sizeof(suites[0]);
    
    // Parse command line arguments
    if (argc > 1) {
        // Disable all tests first
        for (int i = 0; i < num_suites; i++) {
            suites[i].enabled = 0;
        }
        
        // Enable requested tests
        for (int arg = 1; arg < argc; arg++) {
            for (int i = 0; i < num_suites; i++) {
                if (strcmp(argv[arg], suites[i].name) == 0) {
                    suites[i].enabled = 1;
                    break;
                }
            }
        }
    }
    
    printf("Running malloc test suite...\n");
    printf("To run specific tests: %s [malloc|free|realloc|show_mem|performance]\n\n", argv[0]);
    
    // Run all enabled test suites
    for (int i = 0; i < num_suites; i++) {
        run_test_suite(&suites[i]);
        printf("\n");
    }
    
    // Print final summary
    print_summary(suites, num_suites);
    
    // Return success if all tests passed
    return g_total_score >= 0 ? 0 : 1;
}
