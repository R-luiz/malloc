#include "../../include/malloc.h"
#include <string.h>

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);
    
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;
    
    // For now, copy a reasonable amount (we don't track original size easily)
    // This is a simple implementation - full version will track sizes properly
    memcpy(new_ptr, ptr, size < 1024 ? size : 1024);
    free(ptr);
    
    return new_ptr;
}
