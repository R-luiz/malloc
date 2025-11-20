#include <pthread.h>
#include <stdio.h>

// Test if pthread mutex initialization causes allocations

// First, declare the mutex WITHOUT initializing it
extern pthread_mutex_t g_mutex;
extern void* malloc(size_t size);
extern void free(void *ptr);
extern int check_malloc_leaks(void);

// Counter to track malloc calls
static int malloc_call_count = 0;

// Our own malloc wrapper to count calls
void* test_malloc(size_t size) {
    malloc_call_count++;
    printf("malloc() called! Count: %d, size: %zu\n", malloc_call_count, size);
    return malloc(size);
}

int main(void) {
    printf("=== Testing pthread mutex initialization ===\n\n");
    
    printf("BEFORE using mutex (checking for early init):\n");
    int leaks_before = check_malloc_leaks();
    printf("Leaks: %d\n\n", leaks_before);
    
    printf("Locking mutex for the first time...\n");
    pthread_mutex_lock(&g_mutex);
    printf("Mutex locked successfully\n");
    
    int leaks_after_lock = check_malloc_leaks();
    printf("Leaks after first lock: %d\n\n", leaks_after_lock);
    
    pthread_mutex_unlock(&g_mutex);
    printf("Mutex unlocked\n");
    
    int leaks_after_unlock = check_malloc_leaks();
    printf("Leaks after unlock: %d\n\n", leaks_after_unlock);
    
    printf("\n=== RESULT ===\n");
    printf("Leaks introduced by mutex operations: %d\n", leaks_after_lock - leaks_before);
    
    return 0;
}
