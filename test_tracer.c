#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>

// Hook into malloc to trace calls during initialization

static int init_done = 0;
static int call_count = 0;

// Get the real malloc
static void* (*real_malloc)(size_t) = NULL;

void __attribute__((constructor)) init_tracer(void) {
    fprintf(stderr, "=== TRACER: Constructor called (before main) ===\n");
}

void* malloc(size_t size) {
    if (!real_malloc) {
        // First time - get the real malloc
        real_malloc = dlsym(RTLD_NEXT, "malloc");
        if (!real_malloc) {
            fprintf(stderr, "TRACER: Cannot find real malloc!\n");
            return NULL;
        }
    }
    
    call_count++;
    
    if (!init_done && call_count <= 10) {
        // Print backtrace info for first few calls
        fprintf(stderr, "TRACER: malloc(%zu) call #%d\n", size, call_count);
        
        // Print backtrace
        void *buffer[10];
        int nptrs = backtrace(buffer, 10);
        char **symbols = backtrace_symbols(buffer, nptrs);
        if (symbols) {
            for (int i = 0; i < nptrs; i++) {
                fprintf(stderr, "  [%d] %s\n", i, symbols[i]);
            }
            free(symbols);  // This uses real free
        }
    }
    
    return real_malloc(size);
}

void __attribute__((constructor)) mark_init_done(void) {
    fprintf(stderr, "=== TRACER: Init done, entering main ===\n");
    init_done = 1;
}

int main(void) {
    fprintf(stderr, "\n=== IN MAIN ===\n");
    fprintf(stderr, "Total malloc calls during init: %d\n", call_count);
    return 0;
}
