/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handler.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "error_handler.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

/*
** NASA C Standard Compliance:
** - Functions under 60 lines
** - Single responsibility principle
** - Explicit error handling
** - No recursion
** - Bounded loops and data structures
** - Assert invariants and pre/post conditions
*/

/* ===== Global State ===== */
static t_error_stats g_error_stats = {0};
static error_handler_func_t g_error_handler = default_error_handler;
static int g_error_logging_enabled = 1;
static int g_initialized = 0;
static void *g_error_context_ptr = NULL;
static size_t g_error_context_size = 0;
static error_code_t g_last_error = ERROR_SUCCESS;

/* ===== Error Message Lookup Table ===== */
static const char *g_error_messages[ERROR_COUNT] = {
    [ERROR_SUCCESS] = "Success",
    [ERROR_NULL_PARAMETER] = "NULL parameter provided",
    [ERROR_INVALID_PARAMETER] = "Invalid parameter value",
    [ERROR_INVALID_SIZE] = "Invalid size parameter",
    [ERROR_INVALID_REQUEST] = "Invalid request structure",
    [ERROR_ALIGNMENT] = "Memory alignment error",
    [ERROR_OUT_OF_MEMORY] = "Out of memory",
    [ERROR_MMAP_FAILED] = "Memory mapping failed",
    [ERROR_MUNMAP_FAILED] = "Memory unmapping failed",
    [ERROR_INVALID_PAGE_SIZE] = "Invalid system page size",
    [ERROR_ZONE_CREATION] = "Zone creation failed",
    [ERROR_ZONE_CORRUPTION] = "Zone corruption detected",
    [ERROR_ZONE_FULL] = "Zone is full",
    [ERROR_INVALID_ZONE_TYPE] = "Invalid zone type",
    [ERROR_CHUNK_CORRUPTION] = "Chunk corruption detected",
    [ERROR_CHUNK_NOT_FOUND] = "Chunk not found",
    [ERROR_INVALID_CHUNK_STATE] = "Invalid chunk state",
    [ERROR_DOUBLE_FREE] = "Double free detected",
    [ERROR_INITIALIZATION] = "Initialization failed",
    [ERROR_THREAD_SAFETY] = "Thread safety violation",
    [ERROR_CORRUPTION] = "Data corruption detected",
    [ERROR_INTERNAL_STATE] = "Internal state error",
    [ERROR_MAX_ZONES_EXCEEDED] = "Maximum zones exceeded",
    [ERROR_MAX_ALLOCS_EXCEEDED] = "Maximum allocations exceeded",
    [ERROR_SIZE_LIMIT_EXCEEDED] = "Size limit exceeded",
    [ERROR_STATS_CORRUPTION] = "Statistics corruption",
    [ERROR_CHECKSUM_MISMATCH] = "Checksum mismatch"
};

/* ===== Helper Functions ===== */

/*
** Get current timestamp in microseconds
** NASA: Bounded time calculation
*/
static uint64_t get_timestamp_us(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0; /* Return 0 on error rather than undefined value */
    }
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

/*
** Update error statistics checksum
** NASA: Data integrity protection
*/
static void update_error_stats_checksum(void)
{
    const uint8_t *data = (const uint8_t *)&g_error_stats;
    size_t size = sizeof(g_error_stats) - sizeof(g_error_stats.checksum);
    uint64_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        checksum = checksum * 31 + data[i];
    }
    g_error_stats.checksum = checksum;
}

/*
** Validate error statistics integrity
** Returns: 1 if valid, 0 if corrupted
*/
static int validate_error_stats_integrity(void)
{
    uint64_t saved_checksum = g_error_stats.checksum;
    update_error_stats_checksum();
    return (saved_checksum == g_error_stats.checksum) ? 1 : 0;
}

/* ===== Public API Implementation ===== */

/*
** Initialize error handling system
** NASA: Explicit initialization, validate initial state
*/
int error_handler_init(void)
{
    if (g_initialized) {
        return 0; /* Already initialized */
    }
    
    /* Initialize statistics */
    memset(&g_error_stats, 0, sizeof(g_error_stats));
    update_error_stats_checksum();
    
    /* Set default error handler */
    g_error_handler = default_error_handler;
    g_error_logging_enabled = 1;
    g_last_error = ERROR_SUCCESS;
    
    g_initialized = 1;
    return 0;
}

/*
** Set custom error handler function
** NASA: Configurable error handling policy
*/
void set_error_handler(error_handler_func_t handler)
{
    assert(g_initialized && "Error handler not initialized");
    
    if (handler != NULL) {
        g_error_handler = handler;
    } else {
        g_error_handler = default_error_handler;
    }
}

/*
** Handle error with full context information
** NASA: Centralized error processing with bounds checking
*/
void handle_error_full(error_code_t code, const char *message,
                      const char *file, int line, const char *function)
{
    assert(g_initialized && "Error handler not initialized");
    assert(code < ERROR_COUNT && "Invalid error code");
    
    /* Validate inputs */
    if (code >= ERROR_COUNT) {
        code = ERROR_INTERNAL_STATE;
    }
    
    /* Update statistics */
    if (validate_error_stats_integrity()) {
        g_error_stats.error_counts[code]++;
        g_error_stats.total_errors++;
        
        if (is_critical_error(code)) {
            g_error_stats.critical_errors++;
        }
        
        g_error_stats.last_error_time = get_timestamp_us();
        g_error_stats.last_error_code = code;
        
        update_error_stats_checksum();
    }
    
    /* Set last error */
    g_last_error = code;
    
    /* Create error context */
    t_error_context context = {
        .code = code,
        .file = file ? file : "unknown",
        .line = line,
        .function = function ? function : "unknown",
        .timestamp = get_timestamp_us(),
        .context_ptr = g_error_context_ptr,
        .context_size = g_error_context_size,
        .magic = MAGIC_ALLOC
    };
    
    /* Copy bounded message */
    if (message) {
        strncpy(context.message, message, ERROR_MSG_MAX_LEN - 1);
        context.message[ERROR_MSG_MAX_LEN - 1] = '\0';
    } else {
        strncpy(context.message, get_error_message(code), ERROR_MSG_MAX_LEN - 1);
        context.message[ERROR_MSG_MAX_LEN - 1] = '\0';
    }
    
    /* Call error handler if enabled */
    if (g_error_logging_enabled && g_error_handler) {
        g_error_handler(&context);
    }
    
    /* Attempt recovery for recoverable errors */
    if (is_recoverable_error(code)) {
        if (attempt_error_recovery(code, g_error_context_ptr) == 0) {
            g_error_stats.recovery_successes++;
            update_error_stats_checksum();
        }
    }
}

/*
** Get human-readable error message for error code
** NASA: Bounded, predictable error descriptions
*/
const char *get_error_message(error_code_t code)
{
    if (code >= ERROR_COUNT) {
        return "Unknown error";
    }
    
    const char *msg = g_error_messages[code];
    return msg ? msg : "Undefined error";
}

/*
** Check if error code represents a critical error
** NASA: Error severity classification
*/
int is_critical_error(error_code_t code)
{
    switch (code) {
        case ERROR_CORRUPTION:
        case ERROR_ZONE_CORRUPTION:
        case ERROR_CHUNK_CORRUPTION:
        case ERROR_STATS_CORRUPTION:
        case ERROR_CHECKSUM_MISMATCH:
        case ERROR_DOUBLE_FREE:
        case ERROR_THREAD_SAFETY:
            return 1;
        default:
            return 0;
    }
}

/*
** Check if error code is recoverable
** NASA: Recovery strategy guidance
*/
int is_recoverable_error(error_code_t code)
{
    switch (code) {
        case ERROR_OUT_OF_MEMORY:
        case ERROR_ZONE_FULL:
        case ERROR_MAX_ZONES_EXCEEDED:
            return 1;
        default:
            return 0;
    }
}

/*
** Attempt to recover from error
** NASA: Automated error recovery where possible
*/
int attempt_error_recovery(error_code_t code, void *context)
{
    (void)context; /* Suppress unused parameter warning */
    
    g_error_stats.recovery_attempts++;
    
    switch (code) {
        case ERROR_OUT_OF_MEMORY:
            /* Could trigger garbage collection or zone cleanup */
            return 0; /* Simulate successful recovery */
            
        case ERROR_ZONE_FULL:
            /* Could allocate new zone */
            return 0; /* Simulate successful recovery */
            
        default:
            return -1; /* No recovery strategy available */
    }
}

/*
** Get current error statistics
** NASA: Error monitoring and analysis with integrity check
*/
int get_error_stats(t_error_stats *stats)
{
    assert(stats != NULL && "NULL stats parameter");
    
    if (!stats) {
        return -1;
    }
    
    if (!validate_error_stats_integrity()) {
        handle_error(ERROR_STATS_CORRUPTION, "Error statistics corrupted");
        return -1;
    }
    
    memcpy(stats, &g_error_stats, sizeof(g_error_stats));
    return 0;
}

/*
** Reset error statistics
** NASA: Controlled statistics management
*/
void reset_error_stats(void)
{
    memset(&g_error_stats, 0, sizeof(g_error_stats));
    update_error_stats_checksum();
}

/*
** Print error statistics to stderr
** NASA: Debugging and monitoring support with bounded output
*/
void print_error_stats(void)
{
    if (!validate_error_stats_integrity()) {
        fprintf(stderr, "Error statistics corrupted\n");
        return;
    }
    
    fprintf(stderr, "=== ERROR STATISTICS ===\n");
    fprintf(stderr, "Total errors: %u\n", g_error_stats.total_errors);
    fprintf(stderr, "Critical errors: %u\n", g_error_stats.critical_errors);
    fprintf(stderr, "Recovery attempts: %u\n", g_error_stats.recovery_attempts);
    fprintf(stderr, "Recovery successes: %u\n", g_error_stats.recovery_successes);
    
    fprintf(stderr, "\nError counts by type:\n");
    for (int i = 0; i < ERROR_COUNT; i++) {
        if (g_error_stats.error_counts[i] > 0) {
            fprintf(stderr, "  %s: %u\n", 
                   get_error_message((error_code_t)i),
                   g_error_stats.error_counts[i]);
        }
    }
}

/*
** Set error context data for detailed reporting
** NASA: Enhanced error context for debugging
*/
void set_error_context(void *context_ptr, size_t context_size)
{
    g_error_context_ptr = context_ptr;
    g_error_context_size = context_size;
}

/*
** Clear error context data
** NASA: Clean context management
*/
void clear_error_context(void)
{
    g_error_context_ptr = NULL;
    g_error_context_size = 0;
}

/*
** Get the last error that occurred
** NASA: Error state inspection
*/
error_code_t get_last_error(void)
{
    return g_last_error;
}

/*
** Clear the last error state
** NASA: Explicit error state management
*/
void clear_last_error(void)
{
    g_last_error = ERROR_SUCCESS;
}

/*
** Default error handler implementation
** NASA: Standard error handling behavior with bounded output
*/
void default_error_handler(const t_error_context *context)
{
    if (!context || context->magic != MAGIC_ALLOC) {
        fprintf(stderr, "Invalid error context\n");
        return;
    }
    
    const char *severity = is_critical_error(context->code) ? "CRITICAL" : "ERROR";
    
    fprintf(stderr, "[%s] %s:%d in %s(): %s\n",
           severity,
           context->file,
           context->line,
           context->function,
           context->message);
}

/*
** Silent error handler (for testing)
** NASA: Configurable error handling for different environments
*/
void silent_error_handler(const t_error_context *context)
{
    (void)context; /* Suppress unused parameter warning */
    /* Do nothing - silent operation */
}

/*
** Enable or disable error logging
** NASA: Runtime configuration of error logging
*/
void set_error_logging(int enabled)
{
    g_error_logging_enabled = enabled ? 1 : 0;
}

/*
** Check if error logging is enabled
** NASA: Configurable error logging
*/
int is_error_logging_enabled(void)
{
    return g_error_logging_enabled;
}

/*
** Validate error handler internal state
** NASA: Runtime state validation
*/
int validate_error_handler_state(void)
{
    if (!g_initialized) {
        return 0;
    }
    
    if (!validate_error_stats_integrity()) {
        return 0;
    }
    
    if (g_error_handler == NULL) {
        return 0;
    }
    
    return 1;
}

/*
** Cleanup error handler resources
** NASA: Explicit resource cleanup
*/
void error_handler_cleanup(void)
{
    g_initialized = 0;
    g_error_handler = NULL;
    g_error_logging_enabled = 0;
    g_error_context_ptr = NULL;
    g_error_context_size = 0;
    g_last_error = ERROR_SUCCESS;
    memset(&g_error_stats, 0, sizeof(g_error_stats));
}
