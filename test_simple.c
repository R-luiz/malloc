#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Testing malloc...\n");
    
    void *ptr1 = malloc(100);  // TINY
    if (ptr1) {
        printf("TINY allocation (100 bytes): SUCCESS\n");
        free(ptr1);
        printf("TINY free: SUCCESS\n");
    }
    
    void *ptr2 = malloc(500);  // SMALL
    if (ptr2) {
        printf("SMALL allocation (500 bytes): SUCCESS\n");
        free(ptr2);
        printf("SMALL free: SUCCESS\n");
    }
    
    void *ptr3 = malloc(2000); // LARGE
    if (ptr3) {
        printf("LARGE allocation (2000 bytes): SUCCESS\n");
        free(ptr3);
        printf("LARGE free: SUCCESS\n");
    }
    
    printf("All tests completed!\n");
    return 0;
}