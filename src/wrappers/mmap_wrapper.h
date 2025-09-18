/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mmap_wrapper.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MMAP_WRAPPER_H
# define MMAP_WRAPPER_H

# include <sys/mman.h>
# include <stddef.h>
# include <stdint.h>
# include "malloc_constants.h"

/*
** NASA C Standard Compliance:
** - All system calls wrapped and validated
** - Error conditions explicitly handled
** - Resource tracking and statistics
** - No direct syscall usage in main code
*/

/* ===== mmap Statistics Structure ===== */
typedef struct {
    size_t      total_mapped;       /* Total bytes mapped */
    size_t      total_unmapped;     /* Total bytes unmapped */
    size_t      current_mapped;     /* Currently mapped bytes */
    uint32_t    map_count;          /* Number of mmap calls */
    uint32_t    unmap_count;        /* Number of munmap calls */
    uint32_t    map_failures;       /* Failed mmap calls */
    uint32_t    unmap_failures;     /* Failed munmap calls */
    size_t      peak_mapped;        /* Peak mapped memory */
    uint64_t    checksum;           /* Statistics integrity */
} t_mmap_stats;

/* ===== Memory Mapping Request Structure ===== */
typedef struct {
    size_t      size;               /* Requested size */
    size_t      aligned_size;       /* Page-aligned size */
    int         protection;         /* Memory protection flags */
    int         flags;              /* Mapping flags */
    uint32_t    magic;              /* Request validation */
} t_mmap_request;

/* ===== Memory Mapping Result Structure ===== */
typedef struct {
    void        *addr;              /* Mapped address */
    size_t      size;               /* Actual mapped size */
    int         error_code;         /* Error code (0 = success) */
    uint32_t    magic;              /* Result validation */
} t_mmap_result;

/* ===== Function Prototypes ===== */

/*
** Initialize mmap wrapper system
** Returns: 0 on success, -1 on failure
** NASA: Initialize all subsystems explicitly
*/
int         mmap_wrapper_init(void);

/*
** Safe memory mapping with validation
** Pre-conditions: size > 0, size <= MAX_ALLOC_SIZE
** Post-conditions: Returns valid aligned pointer or NULL
** NASA: All parameters validated, bounded operation
*/
void        *safe_mmap(size_t size);

/*
** Safe memory unmapping with validation
** Pre-conditions: addr != NULL, addr from safe_mmap, size > 0
** Post-conditions: Memory unmapped, stats updated
** NASA: Validates all parameters, graceful error handling
*/
int         safe_munmap(void *addr, size_t size);

/*
** Validate mmap request parameters
** Returns: 1 if valid, 0 if invalid
** NASA: Explicit parameter validation
*/
int         validate_mmap_request(const t_mmap_request *request);

/*
** Get current mmap statistics
** Pre-conditions: stats != NULL
** Post-conditions: stats filled with current values
** NASA: Bounded data collection, integrity checking
*/
int         get_mmap_stats(t_mmap_stats *stats);

/*
** Reset mmap statistics
** NASA: Controlled state management
*/
void        reset_mmap_stats(void);

/*
** Check system page size and alignment
** Returns: system page size
** NASA: System parameter discovery and validation
*/
size_t      get_system_page_size(void);

/*
** Align size to page boundary
** Pre-conditions: size > 0
** Post-conditions: Returns page-aligned size
** NASA: Explicit alignment calculation
*/
size_t      align_to_page_size(size_t size);

/*
** Validate mapped memory region
** Returns: 1 if valid, 0 if invalid
** NASA: Runtime validation of memory state
*/
int         validate_mapped_region(void *addr, size_t size);

/*
** Get memory protection flags for zone type
** NASA: Centralized policy for memory protection
*/
int         get_protection_flags(void);

/*
** Get memory mapping flags for zone type
** NASA: Centralized policy for memory mapping
*/
int         get_mapping_flags(void);

/*
** Check if address is properly aligned
** Returns: 1 if aligned, 0 if not
** NASA: Alignment verification
*/
int         is_page_aligned(const void *addr);

/*
** Calculate optimal mapping size for request
** Pre-conditions: requested_size > 0
** Post-conditions: Returns optimal page-aligned size
** NASA: Efficient resource utilization
*/
size_t      calculate_optimal_mapping_size(size_t requested_size);

/* ===== Error Handling Functions ===== */

/*
** Handle mmap error conditions
** NASA: Centralized error processing
*/
void        handle_mmap_error(int error_code, size_t size);

/*
** Handle munmap error conditions
** NASA: Centralized error processing
*/
void        handle_munmap_error(int error_code, void *addr, size_t size);

/*
** Validate mmap wrapper internal state
** Returns: 1 if valid, 0 if corrupted
** NASA: Runtime state validation
*/
int         validate_mmap_wrapper_state(void);

/* ===== Debug and Monitoring Functions ===== */

/*
** Print mmap statistics to stderr
** NASA: Debugging support with bounded output
*/
void        print_mmap_stats(void);

/*
** Check for memory leaks in mmap wrapper
** Returns: 0 if no leaks, positive number of leaks detected
** NASA: Resource leak detection
*/
int         check_mmap_leaks(void);

/*
** Cleanup mmap wrapper resources
** NASA: Explicit resource cleanup
*/
void        mmap_wrapper_cleanup(void);

#endif
