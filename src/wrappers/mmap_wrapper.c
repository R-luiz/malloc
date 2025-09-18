/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mmap_wrapper.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "mmap_wrapper.h"
#include "error_handler.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - No recursion
** - Bounded loops with MAX_ITERATIONS
** - Assert invariants and pre/post conditions
*/

/* ===== Global State ===== */
static t_mmap_stats g_mmap_stats = {0};
static size_t g_page_size = 0;
static int g_initialized = 0;

/* ===== Helper Functions ===== */

/*
** Update statistics checksum for integrity
** NASA: Data integrity protection
*/
static void update_stats_checksum(void)
{
    const uint8_t *data = (const uint8_t *)&g_mmap_stats;
    size_t size = sizeof(g_mmap_stats) - sizeof(g_mmap_stats.checksum);
    uint64_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        checksum = checksum * 31 + data[i];
    }
    g_mmap_stats.checksum = checksum;
}

/*
** Validate statistics integrity
** Returns: 1 if valid, 0 if corrupted
*/
static int validate_stats_integrity(void)
{
    uint64_t saved_checksum = g_mmap_stats.checksum;
    update_stats_checksum();
    return (saved_checksum == g_mmap_stats.checksum) ? 1 : 0;
}

/* ===== Public API Implementation ===== */

/*
** Initialize mmap wrapper system
** NASA: Explicit initialization, validate system parameters
*/
int mmap_wrapper_init(void)
{
    if (g_initialized) {
        return 0; /* Already initialized */
    }
    
    /* Get system page size */
    g_page_size = get_system_page_size();
    if (g_page_size == 0 || g_page_size > (1024 * 1024)) {
        handle_error(ERROR_INVALID_PAGE_SIZE, "Invalid system page size");
        return -1;
    }
    
    /* Initialize statistics */
    memset(&g_mmap_stats, 0, sizeof(g_mmap_stats));
    update_stats_checksum();
    
    g_initialized = 1;
    return 0;
}

/*
** Get system page size with multi-platform support
** NASA: System parameter discovery with fallback
*/
size_t get_system_page_size(void)
{
    size_t page_size;
    
#ifdef LINUX
    long size = sysconf(_SC_PAGESIZE);
    if (size <= 0) {
        return 4096; /* Fallback */
    }
    page_size = (size_t)size;
#elif MACOS
    int size = getpagesize();
    if (size <= 0) {
        return 4096; /* Fallback */
    }
    page_size = (size_t)size;
#else
    page_size = 4096; /* Default fallback */
#endif
    
    /* Validate page size is power of 2 and reasonable */
    if (page_size < 1024 || page_size > (1024 * 1024) ||
        (page_size & (page_size - 1)) != 0) {
        return 4096; /* Safe fallback */
    }
    
    return page_size;
}

/*
** Align size to page boundary
** NASA: Explicit alignment calculation with bounds checking
*/
size_t align_to_page_size(size_t size)
{
    assert(g_initialized && "mmap_wrapper not initialized");
    assert(size > 0 && "Invalid size parameter");
    assert(g_page_size > 0 && "Invalid page size");
    
    if (size > MAX_ALLOC_SIZE) {
        return 0; /* Size too large */
    }
    
    size_t aligned = ((size + g_page_size - 1) / g_page_size) * g_page_size;
    
    /* Ensure alignment didn't overflow */
    if (aligned < size || aligned > MAX_ALLOC_SIZE) {
        return 0;
    }
    
    return aligned;
}

/*
** Validate mmap request parameters
** NASA: Comprehensive parameter validation
*/
int validate_mmap_request(const t_mmap_request *request)
{
    if (!request) {
        return 0;
    }
    
    if (request->magic != MAGIC_ALLOC) {
        return 0;
    }
    
    if (request->size == 0 || request->size > MAX_ALLOC_SIZE) {
        return 0;
    }
    
    if (request->aligned_size < request->size) {
        return 0;
    }
    
    if (!is_page_aligned((void *)(uintptr_t)request->aligned_size)) {
        return 0;
    }
    
    return 1;
}

/*
** Safe memory mapping with comprehensive validation
** NASA: All parameters validated, bounded operation
*/
void *safe_mmap(size_t size)
{
    assert(g_initialized && "mmap_wrapper not initialized");
    assert(size > 0 && "Invalid size parameter");
    
    /* Pre-condition validation */
    if (!IS_VALID_SIZE(size)) {
        handle_error(ERROR_INVALID_SIZE, "Invalid allocation size");
        return NULL;
    }
    
    /* Create and validate request */
    t_mmap_request request = {
        .size = size,
        .aligned_size = align_to_page_size(size),
        .protection = get_protection_flags(),
        .flags = get_mapping_flags(),
        .magic = MAGIC_ALLOC
    };
    
    if (!validate_mmap_request(&request)) {
        handle_error(ERROR_INVALID_REQUEST, "Invalid mmap request");
        return NULL;
    }
    
    /* Perform mmap system call */
    void *addr = mmap(NULL, request.aligned_size, request.protection,
                      request.flags, -1, 0);
    
    if (addr == MAP_FAILED) {
        handle_mmap_error(errno, size);
        g_mmap_stats.map_failures++;
        update_stats_checksum();
        return NULL;
    }
    
    /* Post-condition validation */
    if (!is_page_aligned(addr)) {
        munmap(addr, request.aligned_size);
        handle_error(ERROR_ALIGNMENT, "mmap returned unaligned address");
        return NULL;
    }
    
    /* Update statistics */
    g_mmap_stats.total_mapped += request.aligned_size;
    g_mmap_stats.current_mapped += request.aligned_size;
    g_mmap_stats.map_count++;
    
    if (g_mmap_stats.current_mapped > g_mmap_stats.peak_mapped) {
        g_mmap_stats.peak_mapped = g_mmap_stats.current_mapped;
    }
    
    update_stats_checksum();
    
    return addr;
}

/*
** Safe memory unmapping with validation
** NASA: Validates all parameters, graceful error handling
*/
int safe_munmap(void *addr, size_t size)
{
    assert(g_initialized && "mmap_wrapper not initialized");
    assert(addr != NULL && "NULL address parameter");
    assert(size > 0 && "Invalid size parameter");
    
    /* Pre-condition validation */
    if (!addr || size == 0) {
        handle_error(ERROR_INVALID_PARAMETER, "Invalid munmap parameters");
        return -1;
    }
    
    if (!is_page_aligned(addr)) {
        handle_error(ERROR_ALIGNMENT, "Address not page aligned");
        return -1;
    }
    
    size_t aligned_size = align_to_page_size(size);
    if (aligned_size == 0) {
        handle_error(ERROR_INVALID_SIZE, "Invalid size for munmap");
        return -1;
    }
    
    /* Perform munmap system call */
    int result = munmap(addr, aligned_size);
    
    if (result != 0) {
        handle_munmap_error(errno, addr, size);
        g_mmap_stats.unmap_failures++;
        update_stats_checksum();
        return -1;
    }
    
    /* Update statistics */
    g_mmap_stats.total_unmapped += aligned_size;
    g_mmap_stats.current_mapped -= aligned_size;
    g_mmap_stats.unmap_count++;
    
    update_stats_checksum();
    
    return 0;
}

/*
** Get memory protection flags
** NASA: Centralized policy for memory protection
*/
int get_protection_flags(void)
{
    return PROT_READ | PROT_WRITE;
}

/*
** Get memory mapping flags
** NASA: Centralized policy for memory mapping
*/
int get_mapping_flags(void)
{
    return MAP_PRIVATE | MAP_ANONYMOUS;
}

/*
** Check if address is properly aligned
** NASA: Alignment verification
*/
int is_page_aligned(const void *addr)
{
    if (!addr || g_page_size == 0) {
        return 0;
    }
    
    uintptr_t ptr_val = (uintptr_t)addr;
    return (ptr_val % g_page_size) == 0;
}

/*
** Get current mmap statistics
** NASA: Bounded data collection, integrity checking
*/
int get_mmap_stats(t_mmap_stats *stats)
{
    assert(stats != NULL && "NULL stats parameter");
    
    if (!stats) {
        return -1;
    }
    
    if (!validate_stats_integrity()) {
        handle_error(ERROR_CORRUPTION, "mmap statistics corrupted");
        return -1;
    }
    
    memcpy(stats, &g_mmap_stats, sizeof(g_mmap_stats));
    return 0;
}

/*
** Reset mmap statistics
** NASA: Controlled state management
*/
void reset_mmap_stats(void)
{
    memset(&g_mmap_stats, 0, sizeof(g_mmap_stats));
    update_stats_checksum();
}

/*
** Handle mmap error conditions
** NASA: Centralized error processing
*/
void handle_mmap_error(int error_code, size_t size)
{
    char error_msg[ERROR_MSG_MAX_LEN];
    
    switch (error_code) {
        case ENOMEM:
            snprintf(error_msg, sizeof(error_msg),
                    "Out of memory for mmap size %zu", size);
            break;
        case EINVAL:
            snprintf(error_msg, sizeof(error_msg),
                    "Invalid mmap parameters for size %zu", size);
            break;
        default:
            snprintf(error_msg, sizeof(error_msg),
                    "mmap failed with error %d for size %zu", error_code, size);
            break;
    }
    
    handle_error(ERROR_MMAP_FAILED, error_msg);
}

/*
** Handle munmap error conditions
** NASA: Centralized error processing
*/
void handle_munmap_error(int error_code, void *addr, size_t size)
{
    char error_msg[ERROR_MSG_MAX_LEN];
    
    snprintf(error_msg, sizeof(error_msg),
            "munmap failed with error %d for addr %p size %zu",
            error_code, addr, size);
    
    handle_error(ERROR_MUNMAP_FAILED, error_msg);
}

/*
** Print mmap statistics to stderr
** NASA: Debugging support with bounded output
*/
void print_mmap_stats(void)
{
    if (!validate_stats_integrity()) {
        fprintf(stderr, "mmap statistics corrupted\n");
        return;
    }
    
    fprintf(stderr, "=== MMAP STATISTICS ===\n");
    fprintf(stderr, "Total mapped: %zu bytes\n", g_mmap_stats.total_mapped);
    fprintf(stderr, "Total unmapped: %zu bytes\n", g_mmap_stats.total_unmapped);
    fprintf(stderr, "Currently mapped: %zu bytes\n", g_mmap_stats.current_mapped);
    fprintf(stderr, "Peak mapped: %zu bytes\n", g_mmap_stats.peak_mapped);
    fprintf(stderr, "Map calls: %u\n", g_mmap_stats.map_count);
    fprintf(stderr, "Unmap calls: %u\n", g_mmap_stats.unmap_count);
    fprintf(stderr, "Map failures: %u\n", g_mmap_stats.map_failures);
    fprintf(stderr, "Unmap failures: %u\n", g_mmap_stats.unmap_failures);
}

/*
** Cleanup mmap wrapper resources
** NASA: Explicit resource cleanup
*/
void mmap_wrapper_cleanup(void)
{
    g_initialized = 0;
    g_page_size = 0;
    memset(&g_mmap_stats, 0, sizeof(g_mmap_stats));
}
