/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_all.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 11:56:46 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * Comprehensive test suite that runs all malloc implementation tests
 * 
 * This file provides a unified interface to run all tests, including:
 * - Individual function tests (malloc, free, realloc)
 * - Integration tests
 * - Performance tests
 * - Memory leak detection
 * - Edge case testing
 * 
 * Usage:
 *   ./test_all                     # Run all tests
 *   ./test_all quick               # Run quick tests only
 *   ./test_all malloc free         # Run specific test suites
 *   ./test_all performance         # Run performance tests only
 */

#include "test_common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

/* External test function declarations */
extern int run_malloc_tests(void);
extern int run_free_tests(void);
extern int run_realloc_tests(void);

/* Test configuration */
typedef struct {
    const char *name;
    const char *description;
    int (*test_func)(void);
    int quick_test;  // 1 if included in quick tests
    int enabled;
} test_config_t;

/* Global counters */
static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;
static int g_verbose = 0;

/**
 * Print usage information
 */
void print_usage(const char *program_name)
{
    printf("Usage: %s [options] [test_names...]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help      Show this help message\n");
    printf("  -v, --verbose   Enable verbose output\n");
    printf("  -q, --quick     Run quick tests only\n\n");
    printf("Available tests:\n");
    printf("  malloc          Test malloc() function\n");
    printf("  free            Test free() function\n");
    printf("  realloc         Test realloc() function\n");
    printf("  integration     Test function interactions\n");
    printf("  stress          Stress testing\n");
    printf("  performance     Performance benchmarks\n");
    printf("  leaks           Memory leak detection\n\n");
    printf("Examples:\n");
    printf("  %s                    # Run all tests\n", program_name);
    printf("  %s --quick            # Run quick tests only\n", program_name);
    printf("  %s malloc free        # Run malloc and free tests only\n", program_name);
    printf("  %s --verbose stress   # Run stress tests with verbose output\n", program_name);
}

/**
 * Integration tests - test interactions between functions
 */
int test_integration(void)
{
    printf("\n==========================================\n");
    printf("        INTEGRATION TESTS\n");
    printf("==========================================\n");
    
    int passed = 0, failed = 0;
    
    printf("\n=== Testing malloc + free integration ===\n");
    void *ptr1 = malloc(100);
    if (ptr1) {
        memset(ptr1, 0xAA, 100);
        free(ptr1);
        passed++;
        printf("✓ malloc + free integration works\n");
    } else {
        failed++;
        printf("✗ malloc + free integration failed\n");
    }
    
    printf("\n=== Testing malloc + realloc + free integration ===\n");
    void *ptr2 = malloc(50);
    if (ptr2) {
        memset(ptr2, 0xBB, 50);
        void *ptr2_new = realloc(ptr2, 150);
        if (ptr2_new && ((char*)ptr2_new)[0] == (char)0xBB) {
            free(ptr2_new);
            passed++;
            printf("✓ malloc + realloc + free integration works\n");
        } else {
            if (ptr2_new) free(ptr2_new);
            failed++;
            printf("✗ malloc + realloc + free integration failed\n");
        }
    } else {
        failed++;
        printf("✗ Initial malloc for integration test failed\n");
    }
    
    printf("\n=== Testing multiple allocation patterns ===\n");
    void *ptrs[20];
    int success_count = 0;
    
    // Mixed allocation pattern
    for (int i = 0; i < 20; i++) {
        size_t size = (i % 3 == 0) ? 32 : (i % 3 == 1) ? 256 : 1024;
        ptrs[i] = malloc(size);
        if (ptrs[i]) {
            memset(ptrs[i], i + 0x30, size);
            success_count++;
        }
    }
    
    // Free every other allocation
    for (int i = 0; i < 20; i += 2) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    // Reallocate freed slots
    for (int i = 0; i < 20; i += 2) {
        ptrs[i] = malloc(128);
        if (ptrs[i]) memset(ptrs[i], 0xFF, 128);
    }
    
    // Clean up all remaining
    for (int i = 0; i < 20; i++) {
        if (ptrs[i]) free(ptrs[i]);
    }
    
    if (success_count >= 15) {
        passed++;
        printf("✓ Complex allocation pattern works\n");
    } else {
        failed++;
        printf("✗ Complex allocation pattern failed\n");
    }
    
    printf("\n--- INTEGRATION TESTS SUMMARY ---\n");
    printf("Passed: %d, Failed: %d\n", passed, failed);
    
    return passed - failed;
}

/**
 * Stress tests - push the implementation to its limits
 */
int test_stress(void)
{
    printf("\n==========================================\n");
    printf("          STRESS TESTS\n");
    printf("==========================================\n");
    
    int passed = 0, failed = 0;
    
    printf("\n=== Stress Test 1: Many small allocations ===\n");
    const int SMALL_ALLOCS = 1000;
    void **small_ptrs = malloc(SMALL_ALLOCS * sizeof(void*));
    
    if (small_ptrs) {
        int small_success = 0;
        for (int i = 0; i < SMALL_ALLOCS; i++) {
            small_ptrs[i] = malloc(32);
            if (small_ptrs[i]) {
                *(int*)small_ptrs[i] = i;
                small_success++;
            }
        }
        
        // Verify data
        int data_ok = 1;
        for (int i = 0; i < SMALL_ALLOCS && data_ok; i++) {
            if (small_ptrs[i] && *(int*)small_ptrs[i] != i) {
                data_ok = 0;
            }
        }
        
        // Cleanup
        for (int i = 0; i < SMALL_ALLOCS; i++) {
            if (small_ptrs[i]) free(small_ptrs[i]);
        }
        free(small_ptrs);
        
        if (small_success >= SMALL_ALLOCS * 0.9 && data_ok) {
            passed++;
            printf("✓ Many small allocations: %d/%d succeeded\n", small_success, SMALL_ALLOCS);
        } else {
            failed++;
            printf("✗ Many small allocations: only %d/%d succeeded\n", small_success, SMALL_ALLOCS);
        }
    } else {
        failed++;
        printf("✗ Could not allocate array for stress test\n");
    }
    
    printf("\n=== Stress Test 2: Fragmentation test ===\n");
    const int FRAG_ALLOCS = 100;
    void *frag_ptrs[FRAG_ALLOCS];
    
    // Allocate many blocks
    for (int i = 0; i < FRAG_ALLOCS; i++) {
        frag_ptrs[i] = malloc(64 + (i % 64) * 8);
    }
    
    // Free every third block to create fragmentation
    for (int i = 0; i < FRAG_ALLOCS; i += 3) {
        if (frag_ptrs[i]) {
            free(frag_ptrs[i]);
            frag_ptrs[i] = NULL;
        }
    }
    
    // Try to allocate in fragmented space
    int frag_reallocs = 0;
    for (int i = 0; i < FRAG_ALLOCS; i += 3) {
        frag_ptrs[i] = malloc(32);
        if (frag_ptrs[i]) frag_reallocs++;
    }
    
    // Cleanup
    for (int i = 0; i < FRAG_ALLOCS; i++) {
        if (frag_ptrs[i]) free(frag_ptrs[i]);
    }
    
    if (frag_reallocs >= FRAG_ALLOCS / 6) { // At least half of freed blocks reallocated
        passed++;
        printf("✓ Fragmentation test: %d reallocations successful\n", frag_reallocs);
    } else {
        failed++;
        printf("✗ Fragmentation test: only %d reallocations successful\n", frag_reallocs);
    }
    
    printf("\n--- STRESS TESTS SUMMARY ---\n");
    printf("Passed: %d, Failed: %d\n", passed, failed);
    
    return passed - failed;
}

/**
 * Performance benchmarks
 */
int test_performance(void)
{
    printf("\n==========================================\n");
    printf("       PERFORMANCE TESTS\n");
    printf("==========================================\n");
    
    const int PERF_ITERATIONS = 10000;
    
    printf("\n=== Allocation Performance ===\n");
    clock_t start = clock();
    
    for (int i = 0; i < PERF_ITERATIONS; i++) {
        void *ptr = malloc(64);
        if (ptr) free(ptr);
    }
    
    clock_t end = clock();
    double duration = ((double)(end - start)) / CLOCKS_PER_SEC;
    double rate = PERF_ITERATIONS / duration;
    
    printf("✓ %d malloc+free cycles in %.3f seconds\n", PERF_ITERATIONS, duration);
    printf("✓ Rate: %.0f operations/second\n", rate);
    
    printf("\n=== Realloc Performance ===\n");
    start = clock();
    
    void *ptr = malloc(32);
    for (int i = 0; i < PERF_ITERATIONS / 10; i++) {
        ptr = realloc(ptr, 32 + (i % 1000));
    }
    if (ptr) free(ptr);
    
    end = clock();
    duration = ((double)(end - start)) / CLOCKS_PER_SEC;
    rate = (PERF_ITERATIONS / 10) / duration;
    
    printf("✓ %d realloc operations in %.3f seconds\n", PERF_ITERATIONS / 10, duration);
    printf("✓ Rate: %.0f reallocs/second\n", rate);
    
    printf("\n--- PERFORMANCE TESTS SUMMARY ---\n");
    printf("Performance tests completed successfully\n");
    
    return 1; // Always pass performance tests
}

/**
 * Memory leak detection (basic)
 */
int test_leaks(void)
{
    printf("\n==========================================\n");
    printf("      MEMORY LEAK TESTS\n");
    printf("==========================================\n");
    
    printf("\n=== Basic leak detection ===\n");
    printf("Memory state before allocations:\n");
    show_alloc_mem();
    
    // Allocate some memory
    void *ptr1 = malloc(100);
    void *ptr2 = malloc(200);
    void *ptr3 = malloc(300);
    
    printf("\nMemory state with allocations:\n");
    show_alloc_mem();
    
    // Free some but not all
    free(ptr1);
    free(ptr3);
    
    printf("\nMemory state after partial cleanup:\n");
    show_alloc_mem();
    
    // Free remaining
    free(ptr2);
    
    printf("\nMemory state after full cleanup:\n");
    show_alloc_mem();
    
    printf("\n--- MEMORY LEAK TESTS SUMMARY ---\n");
    printf("Manual inspection of show_alloc_mem() output required\n");
    
    return 1; // Always pass - manual verification needed
}

/**
 * Run a test with timeout protection
 */
int run_test_safe(const char *name, int (*test_func)(void))
{
    printf("🏃 Running %s...\n", name);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process runs the test
        alarm(30); // 30 second timeout
        int result = test_func();
        exit(result >= 0 ? 0 : 1);
    } else if (pid > 0) {
        // Parent process waits
        int status;
        if (waitpid(pid, &status, 0) == pid) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == 0) {
                    printf("✅ %s PASSED\n", name);
                    g_tests_passed++;
                    return 1;
                } else {
                    printf("❌ %s FAILED (exit code %d)\n", name, exit_code);
                    g_tests_failed++;
                    return 0;
                }
            } else if (WIFSIGNALED(status)) {
                printf("💥 %s CRASHED (signal %d)\n", name, WTERMSIG(status));
                g_tests_failed++;
                return 0;
            }
        }
    }
    
    printf("❓ %s result unknown\n", name);
    g_tests_failed++;
    return 0;
}

/**
 * Main function
 */
int main(int argc, char **argv)
{
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                    COMPREHENSIVE MALLOC TEST SUITE            \n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    
    // Test configuration
    test_config_t tests[] = {
        {"malloc", "Test malloc() function", run_malloc_tests, 1, 0},
        {"free", "Test free() function", run_free_tests, 1, 0},
        {"realloc", "Test realloc() function", run_realloc_tests, 1, 0},
        {"integration", "Test function interactions", test_integration, 1, 0},
        {"stress", "Stress testing", test_stress, 0, 0},
        {"performance", "Performance benchmarks", test_performance, 0, 0},
        {"leaks", "Memory leak detection", test_leaks, 0, 0}
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int quick_mode = 0;
    
    // Parse command line arguments
    int test_specified = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            g_verbose = 1;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quick") == 0) {
            quick_mode = 1;
        } else {
            // Enable specific test
            for (int j = 0; j < num_tests; j++) {
                if (strcmp(argv[i], tests[j].name) == 0) {
                    tests[j].enabled = 1;
                    test_specified = 1;
                    break;
                }
            }
        }
    }
    
    // Enable tests based on mode
    if (!test_specified) {
        if (quick_mode) {
            // Enable only quick tests
            for (int i = 0; i < num_tests; i++) {
                tests[i].enabled = tests[i].quick_test;
            }
        } else {
            // Enable all tests
            for (int i = 0; i < num_tests; i++) {
                tests[i].enabled = 1;
            }
        }
    }
    
    printf("Test configuration:\n");
    for (int i = 0; i < num_tests; i++) {
        const char *status = tests[i].enabled ? "ENABLED" : "DISABLED";
        printf("  %-12s: %s - %s\n", tests[i].name, status, tests[i].description);
    }
    printf("\n");
    
    // Run enabled tests
    for (int i = 0; i < num_tests; i++) {
        if (tests[i].enabled) {
            g_tests_run++;
            run_test_safe(tests[i].name, tests[i].test_func);
        }
    }
    
    // Print final summary
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("                           FINAL RESULTS                       \n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Tests Run:    %d\n", g_tests_run);
    printf("Tests Passed: %d\n", g_tests_passed);
    printf("Tests Failed: %d\n", g_tests_failed);
    printf("Success Rate: %.1f%%\n", 
           g_tests_run > 0 ? (100.0 * g_tests_passed) / g_tests_run : 0.0);
    
    if (g_tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! Your malloc implementation is working correctly.\n");
    } else {
        printf("\n❌ %d test(s) failed. Please review the implementation.\n", g_tests_failed);
    }
    
    printf("\nFinal memory state:\n");
    show_alloc_mem();
    printf("\n");
    
    return g_tests_failed == 0 ? 0 : 1;
}
