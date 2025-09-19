#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    printf("Testing minimal malloc implementation...\n");
    
    // Test 1: Basic allocation
    void *ptr1 = malloc(100);
    if (ptr1) {
        printf("✓ malloc(100) succeeded\n");
        strcpy(ptr1, "Hello, World!");
        printf("✓ Data written successfully: %s\n", (char*)ptr1);
        free(ptr1);
        printf("✓ free() called\n");
    } else {
        printf("✗ malloc(100) failed\n");
        return 1;
    }
    
    // Test 2: Large allocation
    void *ptr2 = malloc(8192);
    if (ptr2) {
        printf("✓ malloc(8192) succeeded\n");
        free(ptr2);
        printf("✓ Large allocation freed\n");
    } else {
        printf("✗ malloc(8192) failed\n");
        return 1;
    }
    
    // Test 3: show_alloc_mem
    printf("\nMemory status:\n");
    show_alloc_mem();
    
    printf("\n✅ All tests passed! Phase 1 malloc is working.\n");
    return 0;
}
