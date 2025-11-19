#include "include/malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <stdarg.h>

#define MAX_ALLOCATIONS 10000
#define LOG_BUFFER_SIZE 1024

typedef struct {
    void *ptr;
    size_t size;
    int id;
    struct timeval alloc_time;
    int freed;
} alloc_record_t;

typedef struct {
    alloc_record_t records[MAX_ALLOCATIONS];
    int count;
    int next_id;
    FILE *log_file;
} test_context_t;

static test_context_t g_ctx = {0};

/* ========================================================================== */
/*                            LOGGING UTILITIES                               */
/* ========================================================================== */

void log_msg(const char *format, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    
    printf("%s", buffer);
    
    if (g_ctx.log_file) {
        fprintf(g_ctx.log_file, "%s", buffer);
        fflush(g_ctx.log_file);
    }
}

void init_logging(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char filename[256];
    
    snprintf(filename, sizeof(filename), 
             "logs_tests_%04d%02d%02d_%02d%02d%02d.log",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
    
    g_ctx.log_file = fopen(filename, "w");
    
    if (g_ctx.log_file) {
        log_msg("=================================================================\n");
        log_msg("  MALLOC INTERACTIVE TEST - LOG FILE\n");
        log_msg("  Date: %s", ctime(&now));
        log_msg("=================================================================\n\n");
    } else {
        printf("Warning: Could not create log file %s\n", filename);
    }
}

void close_logging(void)
{
    if (g_ctx.log_file) {
        log_msg("\n=================================================================\n");
        log_msg("  TEST SESSION ENDED\n");
        log_msg("=================================================================\n");
        fclose(g_ctx.log_file);
        g_ctx.log_file = NULL;
    }
}

/* ========================================================================== */
/*                          ALLOCATION TRACKING                               */
/* ========================================================================== */

int add_allocation(void *ptr, size_t size)
{
    if (g_ctx.count >= MAX_ALLOCATIONS) {
        log_msg("ERROR: Max allocations (%d) reached\n", MAX_ALLOCATIONS);
        return -1;
    }
    
    g_ctx.records[g_ctx.count].ptr = ptr;
    g_ctx.records[g_ctx.count].size = size;
    g_ctx.records[g_ctx.count].id = g_ctx.next_id++;
    gettimeofday(&g_ctx.records[g_ctx.count].alloc_time, NULL);
    g_ctx.records[g_ctx.count].freed = 0;
    
    return g_ctx.count++;
}

alloc_record_t *find_allocation(int id)
{
    for (int i = 0; i < g_ctx.count; i++) {
        if (g_ctx.records[i].id == id && !g_ctx.records[i].freed)
            return &g_ctx.records[i];
    }
    return NULL;
}

void mark_freed(int id)
{
    alloc_record_t *rec = find_allocation(id);
    if (rec)
        rec->freed = 1;
}

/* ========================================================================== */
/*                          PERFORMANCE UTILITIES                             */
/* ========================================================================== */

double get_time_diff(struct timeval *start, struct timeval *end)
{
    return (end->tv_sec - start->tv_sec) + 
           (end->tv_usec - start->tv_usec) / 1000000.0;
}

void benchmark_malloc(size_t size, int count)
{
    struct timeval start, end;
    void *ptrs[1000];
    int actual_count = count > 1000 ? 1000 : count;
    
    log_msg("\n--- BENCHMARK: malloc(%zu) x %d times ---\n", size, actual_count);
    
    gettimeofday(&start, NULL);
    for (int i = 0; i < actual_count; i++) {
        ptrs[i] = malloc(size);
    }
    gettimeofday(&end, NULL);
    
    double total_time = get_time_diff(&start, &end);
    double avg_time = total_time / actual_count;
    
    log_msg("Total time: %.6f seconds\n", total_time);
    log_msg("Average time per malloc: %.9f seconds (%.3f µs)\n", 
            avg_time, avg_time * 1000000);
    log_msg("Operations per second: %.2f\n", actual_count / total_time);
    
    for (int i = 0; i < actual_count; i++) {
        if (ptrs[i]) {
            add_allocation(ptrs[i], size);
        }
    }
}

void benchmark_free(void)
{
    struct timeval start, end;
    int freed_count = 0;
    
    log_msg("\n--- BENCHMARK: free() on all active allocations ---\n");
    
    gettimeofday(&start, NULL);
    for (int i = 0; i < g_ctx.count; i++) {
        if (!g_ctx.records[i].freed) {
            free(g_ctx.records[i].ptr);
            g_ctx.records[i].freed = 1;
            freed_count++;
        }
    }
    gettimeofday(&end, NULL);
    
    double total_time = get_time_diff(&start, &end);
    double avg_time = freed_count > 0 ? total_time / freed_count : 0;
    
    log_msg("Freed %d allocations\n", freed_count);
    log_msg("Total time: %.6f seconds\n", total_time);
    log_msg("Average time per free: %.9f seconds (%.3f µs)\n", 
            avg_time, avg_time * 1000000);
}

/* ========================================================================== */
/*                              TEST FUNCTIONS                                */
/* ========================================================================== */

void cmd_malloc(void)
{
    size_t size;
    int count;
    
    log_msg("\n>>> MALLOC COMMAND <<<\n");
    printf("Enter size (bytes): ");
    if (scanf("%zu", &size) != 1) {
        log_msg("ERROR: Invalid size\n");
        while (getchar() != '\n');
        return;
    }
    
    printf("Enter count (1-1000): ");
    if (scanf("%d", &count) != 1 || count < 1 || count > 1000) {
        log_msg("ERROR: Invalid count (must be 1-1000)\n");
        while (getchar() != '\n');
        return;
    }
    
    log_msg("Allocating %d blocks of %zu bytes...\n", count, size);
    
    int success = 0;
    for (int i = 0; i < count; i++) {
        void *ptr = malloc(size);
        if (ptr) {
            int idx = add_allocation(ptr, size);
            if (idx >= 0) {
                success++;
            } else {
                free(ptr);
                break;
            }
        }
    }
    
    log_msg("Successfully allocated: %d/%d blocks\n", success, count);
    log_msg("Total active allocations: %d\n", g_ctx.count);
}

void cmd_free_id(void)
{
    int id;
    
    log_msg("\n>>> FREE BY ID COMMAND <<<\n");
    printf("Enter allocation ID: ");
    if (scanf("%d", &id) != 1) {
        log_msg("ERROR: Invalid ID\n");
        while (getchar() != '\n');
        return;
    }
    
    alloc_record_t *rec = find_allocation(id);
    if (!rec) {
        log_msg("ERROR: Allocation ID %d not found or already freed\n", id);
        return;
    }
    
    log_msg("Freeing allocation ID %d (size: %zu bytes)\n", id, rec->size);
    free(rec->ptr);
    rec->freed = 1;
    log_msg("Successfully freed\n");
}

void cmd_free_all(void)
{
    int count = 0;
    
    log_msg("\n>>> FREE ALL COMMAND <<<\n");
    
    for (int i = 0; i < g_ctx.count; i++) {
        if (!g_ctx.records[i].freed) {
            free(g_ctx.records[i].ptr);
            g_ctx.records[i].freed = 1;
            count++;
        }
    }
    
    log_msg("Freed %d allocations\n", count);
}

void cmd_realloc(void)
{
    int id;
    size_t new_size;
    
    log_msg("\n>>> REALLOC COMMAND <<<\n");
    printf("Enter allocation ID: ");
    if (scanf("%d", &id) != 1) {
        log_msg("ERROR: Invalid ID\n");
        while (getchar() != '\n');
        return;
    }
    
    printf("Enter new size (bytes): ");
    if (scanf("%zu", &new_size) != 1) {
        log_msg("ERROR: Invalid size\n");
        while (getchar() != '\n');
        return;
    }
    
    alloc_record_t *rec = find_allocation(id);
    if (!rec) {
        log_msg("ERROR: Allocation ID %d not found\n", id);
        return;
    }
    
    log_msg("Reallocating ID %d from %zu to %zu bytes\n", 
            id, rec->size, new_size);
    
    void *new_ptr = realloc(rec->ptr, new_size);
    if (new_ptr) {
        rec->ptr = new_ptr;
        rec->size = new_size;
        log_msg("Successfully reallocated\n");
    } else {
        log_msg("ERROR: realloc failed\n");
    }
}

void cmd_list_allocations(void)
{
    int active = 0;
    
    log_msg("\n>>> ACTIVE ALLOCATIONS <<<\n");
    log_msg("%-6s %-18s %-12s %-10s\n", "ID", "Address", "Size", "Type");
    log_msg("-------------------------------------------------------\n");
    
    for (int i = 0; i < g_ctx.count; i++) {
        if (!g_ctx.records[i].freed) {
            const char *type = "LARGE";
            if (g_ctx.records[i].size <= 128)
                type = "TINY";
            else if (g_ctx.records[i].size <= 1024)
                type = "SMALL";
            
            log_msg("%-6d %p %-12zu %-10s\n",
                    g_ctx.records[i].id,
                    g_ctx.records[i].ptr,
                    g_ctx.records[i].size,
                    type);
            active++;
        }
    }
    
    log_msg("-------------------------------------------------------\n");
    log_msg("Total active: %d\n", active);
}

void cmd_show_alloc_mem(void)
{
    log_msg("\n>>> SHOW_ALLOC_MEM OUTPUT <<<\n");
    show_alloc_mem();
}

void cmd_stats(void)
{
    t_malloc_stats stats;
    
    log_msg("\n>>> MALLOC STATISTICS <<<\n");
    
    if (get_malloc_stats(&stats) == 0) {
        log_msg("Bytes allocated:     %zu\n", stats.bytes_allocated);
        log_msg("TINY allocations:    %u\n", stats.allocs_tiny);
        log_msg("SMALL allocations:   %u\n", stats.allocs_small);
        log_msg("LARGE allocations:   %u\n", stats.allocs_large);
        log_msg("Active zones:        %u\n", stats.zones_active);
        log_msg("Total zones:         %u\n", stats.zones_total);
    } else {
        log_msg("ERROR: Could not retrieve statistics\n");
    }
    
    int tracked_active = 0;
    for (int i = 0; i < g_ctx.count; i++) {
        if (!g_ctx.records[i].freed)
            tracked_active++;
    }
    log_msg("\nTracked allocations: %d active, %d freed, %d total\n",
            tracked_active, g_ctx.count - tracked_active, g_ctx.count);
}

void cmd_check_leaks(void)
{
    log_msg("\n>>> MEMORY LEAK CHECK <<<\n");
    
    int leaks = check_malloc_leaks();
    
    if (leaks > 0) {
        log_msg("WARNING: %d memory leaks detected\n", leaks);
    } else {
        log_msg("No memory leaks detected\n");
    }
}

void cmd_validate_system(void)
{
    log_msg("\n>>> SYSTEM VALIDATION <<<\n");
    
    int result = malloc_validate_system();
    
    if (result == 0) {
        log_msg("System validation: PASS\n");
    } else {
        log_msg("System validation: FAIL (code %d)\n", result);
    }
}

void cmd_benchmark_suite(void)
{
    log_msg("\n>>> BENCHMARK SUITE <<<\n");
    
    benchmark_malloc(64, 100);
    benchmark_malloc(512, 100);
    benchmark_malloc(2048, 100);
    benchmark_free();
}

void cmd_stress_test(void)
{
    int count;
    
    log_msg("\n>>> STRESS TEST <<<\n");
    printf("Enter number of allocations (1-1000): ");
    if (scanf("%d", &count) != 1 || count < 1 || count > 1000) {
        log_msg("ERROR: Invalid count\n");
        while (getchar() != '\n');
        return;
    }
    
    log_msg("Starting stress test with %d allocations...\n", count);
    
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    for (int i = 0; i < count; i++) {
        size_t size = (rand() % 10240) + 1;
        void *ptr = malloc(size);
        if (ptr) {
            add_allocation(ptr, size);
        }
    }
    
    gettimeofday(&end, NULL);
    double elapsed = get_time_diff(&start, &end);
    
    log_msg("Stress test completed in %.6f seconds\n", elapsed);
    log_msg("Average: %.6f seconds per allocation\n", elapsed / count);
}

void cmd_fragmentation_test(void)
{
    log_msg("\n>>> FRAGMENTATION TEST <<<\n");
    log_msg("Allocating alternating sizes...\n");
    
    void *ptrs[100];
    for (int i = 0; i < 100; i++) {
        size_t size = (i % 2) ? 32 : 1024;
        ptrs[i] = malloc(size);
        if (ptrs[i]) {
            add_allocation(ptrs[i], size);
        }
    }
    
    log_msg("Freeing every other allocation...\n");
    for (int i = 0; i < 100; i += 2) {
        if (ptrs[i]) {
            free(ptrs[i]);
            mark_freed(i);
        }
    }
    
    log_msg("Allocating in freed spaces...\n");
    for (int i = 0; i < 50; i++) {
        void *ptr = malloc(64);
        if (ptr) {
            add_allocation(ptr, 64);
        }
    }
    
    log_msg("Fragmentation test completed\n");
    show_alloc_mem();
}

void cmd_clear_screen(void)
{
    printf("\033[2J\033[H");
    log_msg("\n>>> SCREEN CLEARED <<<\n");
}

/* ========================================================================== */
/*                               MENU SYSTEM                                  */
/* ========================================================================== */

void print_menu(void)
{
    printf("\n");
    printf("==============================================================\n");
    printf("                 MALLOC INTERACTIVE TEST                      \n");
    printf("==============================================================\n");
    printf("\n");
    printf("ALLOCATION:\n");
    printf("  1. malloc (size, count)      - Allocate memory blocks\n");
    printf("  2. free (id)                 - Free specific allocation\n");
    printf("  3. free all                  - Free all allocations\n");
    printf("  4. realloc (id, new_size)    - Reallocate memory\n");
    printf("\n");
    printf("VISUALIZATION:\n");
    printf("  5. list allocations          - Show tracked allocations\n");
    printf("  6. show_alloc_mem()          - Display memory zones\n");
    printf("  7. stats                     - Show malloc statistics\n");
    printf("\n");
    printf("DIAGNOSTICS:\n");
    printf("  8. check leaks               - Check for memory leaks\n");
    printf("  9. validate system           - Run system validation\n");
    printf("\n");
    printf("PERFORMANCE:\n");
    printf(" 10. benchmark suite           - Run performance benchmarks\n");
    printf(" 11. stress test               - Random allocation stress\n");
    printf(" 12. fragmentation test        - Test memory fragmentation\n");
    printf("\n");
    printf("UTILITIES:\n");
    printf(" 13. clear screen              - Clear terminal\n");
    printf("  0. exit                      - Exit program\n");
    printf("\n");
    printf("Active allocations: %d/%d\n", g_ctx.count, MAX_ALLOCATIONS);
    printf("==============================================================\n");
    printf("Enter command: ");
}

void execute_command(int cmd)
{
    switch (cmd) {
        case 1:  cmd_malloc(); break;
        case 2:  cmd_free_id(); break;
        case 3:  cmd_free_all(); break;
        case 4:  cmd_realloc(); break;
        case 5:  cmd_list_allocations(); break;
        case 6:  cmd_show_alloc_mem(); break;
        case 7:  cmd_stats(); break;
        case 8:  cmd_check_leaks(); break;
        case 9:  cmd_validate_system(); break;
        case 10: cmd_benchmark_suite(); break;
        case 11: cmd_stress_test(); break;
        case 12: cmd_fragmentation_test(); break;
        case 13: cmd_clear_screen(); break;
        case 0:  log_msg("\n>>> EXIT <<<\n"); break;
        default: log_msg("ERROR: Invalid command\n");
    }
}

/* ========================================================================== */
/*                                  MAIN                                      */
/* ========================================================================== */

int main(void)
{
    srand(time(NULL));
    init_logging();
    
    log_msg("Interactive test session started\n");
    log_msg("Max allocations: %d\n\n", MAX_ALLOCATIONS);
    
    int cmd;
    do {
        print_menu();
        
        if (scanf("%d", &cmd) != 1) {
            log_msg("ERROR: Invalid input\n");
            while (getchar() != '\n');
            cmd = -1;
            continue;
        }
        
        while (getchar() != '\n');
        
        if (cmd != 0) {
            execute_command(cmd);
        }
        
    } while (cmd != 0);
    
    log_msg("\n>>> FINAL STATISTICS <<<\n");
    cmd_stats();
    cmd_check_leaks();
    
    log_msg("\nCleaning up remaining allocations...\n");
    cmd_free_all();
    
    close_logging();
    
    printf("\nTest session ended. Check log file for details.\n");
    
    return 0;
}
