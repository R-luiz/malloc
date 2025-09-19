/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   metadata.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 14:30:00 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 09:38:34 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef METADATA_H
# define METADATA_H

# include <stddef.h>
# include <stdint.h>
# include <sys/types.h>
# include "malloc_constants.h"

/*
** NASA C Standard Compliance:
** - All structures properly aligned
** - Explicit padding where needed
** - Magic numbers for corruption detection
** - Single responsibility principle
*/

/* ===== Forward Declarations ===== */
typedef struct s_chunk t_chunk;
typedef struct s_zone t_zone;
typedef struct s_zone_manager t_zone_manager;

/* ===== Chunk Metadata Structure ===== */
/*
** Each allocated memory chunk has this header
** NASA Standard: All fields have explicit purpose and bounds
*/
struct s_chunk {
    uint32_t        magic;          /* Corruption detection magic number */
    uint32_t        size;           /* Size of data area (excluding header) */
    chunk_state_t   state;          /* CHUNK_FREE, CHUNK_ALLOCATED, CHUNK_CORRUPTED */
    uint32_t        padding1;       /* Explicit padding for alignment */
    
    t_chunk         *next;          /* Next chunk in zone */
    t_chunk         *prev;          /* Previous chunk in zone */
    t_zone          *zone;          /* Parent zone pointer */
    
    uint64_t        checksum;       /* Header integrity checksum */
    
    /* Guard bytes follow immediately after this structure */
    /* User data follows guard bytes */
    /* Trailing guard bytes after user data */
} __attribute__((packed));

/* ===== Zone Metadata Structure ===== */
/*
** Each memory zone (mmap'd region) has this header
** NASA Standard: Clear ownership and state tracking
*/
struct s_zone {
    uint32_t        magic;          /* Zone corruption detection */
    zone_type_t     type;           /* ZONE_TINY, ZONE_SMALL, ZONE_LARGE */
    uint32_t        total_size;     /* Total zone size from mmap */
    uint32_t        used_size;      /* Currently allocated bytes */
    
    void            *start_addr;    /* Start of zone memory */
    void            *end_addr;      /* End of zone memory */
    
    t_chunk         *first_chunk;   /* First chunk in zone */
    t_chunk         *free_list;     /* Head of free chunks list */
    
    t_zone          *next;          /* Next zone of same type */
    t_zone          *prev;          /* Previous zone of same type */
    
    uint32_t        chunk_count;    /* Number of chunks in zone */
    uint32_t        free_count;     /* Number of free chunks */
    
    pid_t           owner_pid;      /* Process that created this zone */
    uint64_t        creation_time;  /* Zone creation timestamp */
    uint64_t        checksum;       /* Zone header integrity */
    
    uint32_t        padding[2];     /* Explicit padding to 64 bytes */
} __attribute__((packed));

/* ===== Zone Manager Structure ===== */
/*
** Global manager for all memory zones
** NASA Standard: Centralized state management
*/
struct s_zone_manager {
    uint32_t        magic;              /* Manager corruption detection */
    uint32_t        initialized;       /* Initialization flag */
    
    t_zone          *zones[ZONE_TYPE_COUNT];  /* Array of zone lists */
    uint32_t        zone_counts[ZONE_TYPE_COUNT]; /* Counts per type */
    
    size_t          page_size;          /* System page size */
    size_t          total_allocated;    /* Total bytes allocated */
    size_t          total_freed;        /* Total bytes freed */
    
    uint32_t        alloc_count;        /* Total allocation calls */
    uint32_t        free_count;         /* Total free calls */
    uint32_t        realloc_count;      /* Total realloc calls */
    
    uint64_t        stats_checksum;     /* Statistics integrity */
    
    /* Thread safety - managed externally with pthread_mutex */
    /* NASA Standard: No complex locking inside structures */
} __attribute__((packed));

/* ===== Allocation Request Structure ===== */
/*
** Request structure for allocation operations
** NASA Standard: Explicit parameter validation
*/
typedef struct {
    size_t          requested_size;     /* Original request size */
    size_t          aligned_size;       /* Aligned size with headers */
    zone_type_t     zone_type;          /* Target zone type */
    uint32_t        flags;              /* Allocation flags */
    
    /* Validation fields */
    uint32_t        magic;              /* Request validation */
    uint32_t        checksum;           /* Parameter integrity */
} t_alloc_request;

/*
** Forward reference to malloc stats (defined in malloc.h)
*/
struct s_malloc_stats;

/* ===== Function Prototypes for Metadata Operations ===== */

/* Chunk operations */
t_chunk     *chunk_create(void *addr, size_t size, t_zone *zone);
int         chunk_validate(const t_chunk *chunk);
int         chunk_set_state(t_chunk *chunk, chunk_state_t state);
size_t      chunk_get_total_size(const t_chunk *chunk);

/* Zone operations */
t_zone      *zone_create(zone_type_t type, size_t size);
int         zone_validate(const t_zone *zone);
int         zone_add_chunk(t_zone *zone, t_chunk *chunk);
t_chunk     *zone_find_free_chunk(t_zone *zone, size_t size);

/* Manager operations */
int         manager_init(t_zone_manager *manager);
int         manager_validate(const t_zone_manager *manager);
int         manager_add_zone(t_zone_manager *manager, t_zone *zone);
t_zone      *manager_find_zone_for_size(t_zone_manager *manager, size_t size);

/* Statistics operations */
int         stats_update(struct s_malloc_stats *stats, const t_zone_manager *manager);
int         stats_validate(const struct s_malloc_stats *stats);

/* Utility functions */
uint64_t    calculate_checksum(const void *data, size_t size);
int         verify_alignment(const void *ptr);
zone_type_t get_zone_type_for_size(size_t size);

#endif
