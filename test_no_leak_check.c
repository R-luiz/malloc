#include <stdio.h>

extern void* malloc(size_t size);
extern void free(void *ptr);

// DON'T call check_malloc_leaks - manually inspect

int main(void) {
    printf("=== Test WITHOUT calling check_malloc_leaks() ===\n\n");
    
    printf("Making first malloc call from user code:\n");
    void *ptr = malloc(100);
    printf("malloc(100) returned: %p\n", ptr);
    
    printf("\nFreeing it:\n");
    free(ptr);
    printf("free() completed\n");
    
    printf("\nAllocating again:\n");
    void *ptr2 = malloc(200);
    printf("malloc(200) returned: %p\n", ptr2);
    
    free(ptr2);
    printf("free() completed\n");
    
    printf("\n=== Done - no check_malloc_leaks() called ===\n");
    return 0;
}
