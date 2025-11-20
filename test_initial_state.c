#include "include/malloc.h"
#include <stdio.h>

int main(void)
{
    printf("Checking for leaks BEFORE any malloc calls:\n");
    int leaks_before = check_malloc_leaks();
    printf("Leaks before any allocation: %d\n\n", leaks_before);
    
    printf("Allocating and freeing one chunk:\n");
    void *ptr = malloc(100);
    printf("After malloc(100): Leaks = %d\n", check_malloc_leaks());
    
    free(ptr);
    printf("After free(): Leaks = %d\n\n", check_malloc_leaks());
    
    printf("Allocating and freeing another chunk:\n");
    void *ptr2 = malloc(100);
    printf("After malloc(100): Leaks = %d\n", check_malloc_leaks());
    
    free(ptr2);
    printf("After free(): Leaks = %d\n", check_malloc_leaks());
    
    return 0;
}
