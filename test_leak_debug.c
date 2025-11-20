#include "include/malloc.h"
#include <stdio.h>
#include <string.h>

void test_each_function(void)
{
    printf("Testing each test individually...\n\n");
    
    // Test 1
    printf("Test 1: Double-Free\n");
    void *p1 = malloc(100);
    free(p1);
    free(p1);
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 2
    printf("Test 2: Invalid Pointer\n");
    int stack_var = 42;
    free(&stack_var);
    free((void *)0xDEADBEEF);
    void *p2 = malloc(100);
    free((char *)p2 + 50);
    free(p2);
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 3
    printf("Test 3: LARGE Zone Cleanup\n");
    void *l1 = malloc(10000);
    void *l2 = malloc(20000);
    free(l1);
    free(l2);
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 4
    printf("Test 4: O(1) Zone Lookup\n");
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        ptrs[i] = malloc(50);
    }
    for (int i = 0; i < 100; i++) {
        free(ptrs[i]);
    }
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 5
    printf("Test 5: Chunk Validation\n");
    void *p5 = malloc(100);
    free(p5);
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 6
    printf("Test 6: Use-After-Free\n");
    void *p6 = malloc(200);
    strcpy(p6, "Test data");
    free(p6);
    void *p6b = malloc(200);
    free(p6b);
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 7
    printf("Test 7: Merge Adjacent\n");
    void *m1 = malloc(100);
    void *m2 = malloc(100);
    void *m3 = malloc(100);
    free(m2);
    free(m1);
    free(m3);
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 8 - THE SUSPECTED CULPRIT
    printf("Test 8: Realloc\n");
    void *r1 = malloc(100);
    strcpy(r1, "Initial data");
    printf("  After malloc(100): Leaks = %d\n", check_malloc_leaks());
    
    void *r2 = realloc(r1, 50);
    printf("  After realloc(ptr, 50): Leaks = %d\n", check_malloc_leaks());
    
    void *r3 = realloc(r2, 200);
    printf("  After realloc(ptr, 200): Leaks = %d\n", check_malloc_leaks());
    
    free(r3);
    printf("  After free: Leaks = %d\n\n", check_malloc_leaks());
    
    // Test 9
    printf("Test 9: Multiple LARGE\n");
    void *large_ptrs[10];
    for (int i = 0; i < 10; i++) {
        large_ptrs[i] = malloc(5000 + i * 1000);
    }
    for (int i = 0; i < 10; i++) {
        free(large_ptrs[i]);
    }
    printf("Leaks: %d\n\n", check_malloc_leaks());
    
    // Test 10
    printf("Test 10: Mixed Operations\n");
    void *tiny = malloc(50);
    void *small = malloc(500);
    void *large = malloc(5000);
    free(large);
    free(tiny);
    free(small);
    free(tiny); // double free
    printf("Leaks: %d\n\n", check_malloc_leaks());
}

int main(void)
{
    test_each_function();
    
    printf("\n======================\n");
    printf("FINAL LEAK COUNT: %d\n", check_malloc_leaks());
    printf("======================\n");
    
    return 0;
}
