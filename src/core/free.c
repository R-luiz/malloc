#include "../../include/malloc.h"
#include "../../include/types.h"
#include <sys/mman.h>
#include <pthread.h>

extern pthread_mutex_t g_malloc_mutex;

void free(void *ptr)
{
    if (!ptr)
        return;
    
    pthread_mutex_lock(&g_malloc_mutex);
    
    // Get the size metadata stored before the user pointer
    void *real_ptr = (char*)ptr - sizeof(size_t);
    size_t total_size = *(size_t*)real_ptr;
    
    // Unmap the memory
    munmap(real_ptr, total_size);
    
    pthread_mutex_unlock(&g_malloc_mutex);
}
