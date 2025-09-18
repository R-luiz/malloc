/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handler.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_HANDLER_H
# define ERROR_HANDLER_H

# include <stddef.h>
# include <stdint.h>
# include "malloc_constants.h"

/*
** NASA C Standard Compliance:
** - Centralized error management
** - Bounded error messages
** - No error code duplication
** - Clear error categorization
** - Recovery strategies where possible
*/

/* ===== Error Code Enumeration ===== */
typedef enum {
    ERROR_SUCCESS = 0,
    
    /* Parameter validation errors */
    ERROR_NULL_PARAMETER = 1,
    ERROR_INVALID_PARAMETER = 2,
    ERROR_INVALID_SIZE = 3,
    ERROR_INVALID_REQUEST = 4,
    ERROR_ALIGNMENT = 5,
    
    /* Memory management errors */
    ERROR_OUT_OF_MEMORY = 10,
    ERROR_MMAP_FAILED = 11,
    ERROR_MUNMAP_FAILED = 12,
    ERROR_INVALID_PAGE_SIZE = 13,
    
    /* Zone management errors */
    ERROR_ZONE_CREATION = 20,
    ERROR_ZONE_CORRUPTION = 21,
    ERROR_ZONE_FULL = 22,
    ERROR_INVALID_ZONE_TYPE = 23,
    
    /* Chunk management errors */
    ERROR_CHUNK_CORRUPTION = 30,
    ERROR_CHUNK_NOT_FOUND = 31,
    ERROR_INVALID_CHUNK_STATE = 32,
    ERROR_DOUBLE_FREE = 33,
    
    /* System errors */
    ERROR_INITIALIZATION = 40,
    ERROR_THREAD_SAFETY = 41,
    ERROR_CORRUPTION = 42,
    ERROR_INTERNAL_STATE = 43,
    
    /* Limits exceeded */
    ERROR_MAX_ZONES_EXCEEDED = 50,
    ERROR_MAX_ALLOCS_EXCEEDED = 51,
    ERROR_SIZE_LIMIT_EXCEEDED = 52,
    
    /* Statistics and monitoring */
    ERROR_STATS_CORRUPTION = 60,
    ERROR_CHECKSUM_MISMATCH = 61,
    
    ERROR_COUNT = 62
} error_code_t;

/* ===== Error Context Structure ===== */
typedef struct {
    error_code_t    code;                           /* Error code */
    char            message[ERROR_MSG_MAX_LEN];     /* Bounded error message */
    const char      *file;                          /* Source file */
    int             line;                           /* Source line */
    const char      *function;                      /* Function name */
    uint64_t        timestamp;                      /* Error timestamp */
    void            *context_ptr;                   /* Additional context */
    size_t          context_size;                   /* Context data size */
    uint32_t        magic;                          /* Structure validation */
} t_error_context;

/* ===== Error Statistics Structure ===== */
typedef struct {
    uint32_t        error_counts[ERROR_COUNT];      /* Count per error type */
    uint32_t        total_errors;                   /* Total error count */
    uint32_t        critical_errors;                /* Critical error count */
    uint64_t        last_error_time;                /* Last error timestamp */
    error_code_t    last_error_code;                /* Last error code */
    uint32_t        recovery_attempts;              /* Recovery attempt count */
    uint32_t        recovery_successes;             /* Successful recoveries */
    uint64_t        checksum;                       /* Statistics integrity */
} t_error_stats;

/* ===== Error Handler Function Type ===== */
typedef void (*error_handler_func_t)(const t_error_context *context);

/* ===== Function Prototypes ===== */

/*
** Initialize error handling system
** Returns: 0 on success, -1 on failure
** NASA: Explicit initialization required
*/
int         error_handler_init(void);

/*
** Set custom error handler function
** NASA: Configurable error handling policy
*/
void        set_error_handler(error_handler_func_t handler);

/*
** Handle error with context information
** NASA: Centralized error processing with full context
*/
void        handle_error_full(error_code_t code, const char *message,
                             const char *file, int line, const char *function);

/*
** Simplified error handling macro
** NASA: Convenient interface for common error handling
*/
#define handle_error(code, message) \
    handle_error_full((code), (message), __FILE__, __LINE__, __func__)

/*
** Get human-readable error message for error code
** Returns: Static string describing the error
** NASA: Bounded, predictable error descriptions
*/
const char  *get_error_message(error_code_t code);

/*
** Check if error code represents a critical error
** Returns: 1 for critical, 0 for non-critical
** NASA: Error severity classification
*/
int         is_critical_error(error_code_t code);

/*
** Check if error code is recoverable
** Returns: 1 if recoverable, 0 if not
** NASA: Recovery strategy guidance
*/
int         is_recoverable_error(error_code_t code);

/*
** Attempt to recover from error
** Returns: 0 on successful recovery, -1 on failure
** NASA: Automated error recovery where possible
*/
int         attempt_error_recovery(error_code_t code, void *context);

/*
** Get current error statistics
** Pre-conditions: stats != NULL
** Post-conditions: stats filled with current values
** NASA: Error monitoring and analysis
*/
int         get_error_stats(t_error_stats *stats);

/*
** Reset error statistics
** NASA: Controlled statistics management
*/
void        reset_error_stats(void);

/*
** Validate error handler internal state
** Returns: 1 if valid, 0 if corrupted
** NASA: Runtime state validation
*/
int         validate_error_handler_state(void);

/*
** Print error statistics to stderr
** NASA: Debugging and monitoring support
*/
void        print_error_stats(void);

/*
** Set error context data for detailed reporting
** NASA: Enhanced error context for debugging
*/
void        set_error_context(void *context_ptr, size_t context_size);

/*
** Clear error context data
** NASA: Clean context management
*/
void        clear_error_context(void);

/*
** Check if error logging is enabled
** Returns: 1 if enabled, 0 if disabled
** NASA: Configurable error logging
*/
int         is_error_logging_enabled(void);

/*
** Enable or disable error logging
** NASA: Runtime configuration of error logging
*/
void        set_error_logging(int enabled);

/*
** Get the last error that occurred
** Returns: Last error code, ERROR_SUCCESS if none
** NASA: Error state inspection
*/
error_code_t get_last_error(void);

/*
** Clear the last error state
** NASA: Explicit error state management
*/
void        clear_last_error(void);

/*
** Default error handler implementation
** NASA: Standard error handling behavior
*/
void        default_error_handler(const t_error_context *context);

/*
** Silent error handler (for testing)
** NASA: Configurable error handling for different environments
*/
void        silent_error_handler(const t_error_context *context);

/*
** Cleanup error handler resources
** NASA: Explicit resource cleanup
*/
void        error_handler_cleanup(void);

#endif
