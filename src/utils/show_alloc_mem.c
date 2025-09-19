#include "../../include/malloc.h"
#include <stdio.h>
#include <unistd.h>

void show_alloc_mem(void)
{
    printf("TINY : 0x0\n");
    printf("Total : 0 bytes\n");
    printf("SMALL : 0x0\n"); 
    printf("Total : 0 bytes\n");
    printf("LARGE : 0x0\n");
    printf("Total : 0 bytes\n");
}
