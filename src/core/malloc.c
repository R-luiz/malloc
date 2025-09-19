#include "../../include/malloc.h"
#include "../../include/types.h"
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

pthread_mutex_t g_malloc_mutex = PTHREAD_MUTEX_INITIALIZER;
t_malloc_stats g_malloc_stats = {0};

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;
    
    pthread_mutex_lock(&g_malloc_mutex);
    
    size_t total = ((size + sizeof(size_t) + 4095) / 4096) * 4096;
    void *ptr = mmap(NULL, total, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (ptr == MAP_FAILED) {
        pthread_mutex_unlock(&g_malloc_mutex);
        return NULL;
    }
    
    *(size_t*)ptr = total;
    pthread_mutex_unlock(&g_malloc_mutex);
    
    return (char*)ptr + sizeof(size_t);
}
