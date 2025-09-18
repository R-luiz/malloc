/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   malloc_constants.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 13:37:29 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MALLOC_CONSTANTS_H
# define MALLOC_CONSTANTS_H

/*
** NASA C Standard Compliance:
** - No magic numbers in code
** - All constants explicitly defined
** - Clear naming conventions
** - Bounded values for safety
*/

/* ===== Size Category Thresholds ===== */
# define TINY_MAX               128
# define SMALL_MAX              1024
# define LARGE_MIN              1025

/* ===== Zone Sizes (aligned to page boundaries) ===== */
# define TINY_ZONE_SIZE         (16 * 1024)      /* 16KB for tiny allocations */
# define SMALL_ZONE_SIZE        (104 * 1024)     /* 104KB for small allocations */

/* ===== Memory Alignment ===== */
# define ALIGNMENT              16               /* 16-byte alignment for performance */
# define ALIGN_MASK             (ALIGNMENT - 1)

/* ===== Magic Numbers for Corruption Detection ===== */
# define MAGIC_ALLOC            0xDEADBEEF      /* Allocated chunk marker */
# define MAGIC_FREE             0xFEEDFACE      /* Free chunk marker */
# define MAGIC_ZONE             0xCAFEBABE      /* Zone header marker */
# define MAGIC_CORRUPTED        0xDEADDEAD      /* Corruption detected */

/* ===== Safety Limits (NASA bounded values) ===== */
# define MAX_ITERATIONS         1000            /* Loop bound protection */
# define MAX_ZONES_PER_TYPE     100             /* Maximum zones per category */
# define MAX_ALLOC_SIZE         (1UL << 30)     /* 1GB maximum allocation */
# define MIN_CHUNK_SIZE         16              /* Minimum chunk size */

/* ===== Error Handling ===== */
# define ERROR_MSG_MAX_LEN      256             /* Bounded error message length */
# define RETRY_MAX_COUNT        3               /* Maximum retry attempts */

/* ===== Chunk Header Sizes ===== */
# define CHUNK_HEADER_SIZE      32              /* Size of chunk metadata */
# define ZONE_HEADER_SIZE       64              /* Size of zone metadata */

/* ===== Memory Protection ===== */
# define GUARD_SIZE             16              /* Guard bytes around allocations */
# define CANARY_VALUE           0xAA            /* Canary byte pattern */

/* ===== Utility Macros ===== */
# define ALIGN_UP(size)         (((size) + ALIGN_MASK) & ~ALIGN_MASK)
# define IS_ALIGNED(ptr)        (((uintptr_t)(ptr) & ALIGN_MASK) == 0)
# define IS_VALID_SIZE(size)    ((size) > 0 && (size) <= MAX_ALLOC_SIZE)

/* ===== Zone Type Enumeration ===== */
typedef enum {
    ZONE_TINY = 0,
    ZONE_SMALL = 1,
    ZONE_LARGE = 2,
    ZONE_TYPE_COUNT = 3
} zone_type_t;

/* ===== Chunk State Enumeration ===== */
typedef enum {
    CHUNK_FREE = 0,
    CHUNK_ALLOCATED = 1,
    CHUNK_CORRUPTED = 2
} chunk_state_t;

#endif
