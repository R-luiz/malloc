#ifndef MALLOC_INTERNAL_H
# define MALLOC_INTERNAL_H

# include "malloc.h"
# include <pthread.h>

# define ALIGNMENT 16
# define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

# define TINY_MAX 128
# define SMALL_MAX 1024
# define TINY_ZONE_SIZE (16 * 4096)
# define SMALL_ZONE_SIZE (104 * 4096)
# define MIN_SPLIT_SIZE 32

# define MAX_ZONES_PER_TYPE 1000
# define MAX_CHUNKS_PER_ZONE 10000
# define MAX_ZONE_SEARCH 100

# define CHUNK_MAGIC_ALLOCATED 0xDEADBEEF
# define CHUNK_MAGIC_FREE 0xFEEDFACE
# define ZONE_MAGIC 0xCAFEBABE

typedef enum {
    ZONE_TINY = 0,
    ZONE_SMALL = 1,
    ZONE_LARGE = 2
} t_zone_type;

typedef struct s_zone t_zone;

typedef struct s_chunk {
    uint32_t magic;
    size_t size;
    int is_free;
    struct s_chunk *next;
    struct s_chunk *prev;
    t_zone *zone;
} t_chunk;

typedef struct s_zone {
    uint32_t magic;
    t_zone_type type;
    size_t total_size;
    size_t used_size;
    void *start;
    void *end;
    t_chunk *chunks;
    struct s_zone *next;
    size_t chunk_count;
} t_zone;

typedef struct {
    t_zone *zones[3];
    size_t zone_counts[3];
} t_zone_manager;

# define CHUNK_HEADER_SIZE ALIGN(sizeof(t_chunk))
# define ZONE_HEADER_SIZE ALIGN(sizeof(t_zone))

extern t_zone_manager g_manager;
extern pthread_mutex_t g_mutex;

t_zone_type get_zone_type(size_t size);
size_t get_zone_size(t_zone_type type);
t_zone *create_zone(t_zone_type type, size_t min_size);
void add_zone_to_manager(t_zone *zone);
void remove_zone_from_manager(t_zone *zone);
t_zone *find_or_create_zone(t_zone_type type, size_t size);
t_zone *find_zone_for_chunk(t_chunk *chunk);
int is_zone_empty(t_zone *zone);

t_chunk *create_chunk_in_zone(t_zone *zone, size_t size);
t_chunk *find_free_chunk(t_zone *zone, size_t size);
void split_chunk(t_chunk *chunk, size_t size, t_zone *zone);
void merge_adjacent_chunks(t_chunk *chunk, t_zone *zone);
void *get_user_ptr(t_chunk *chunk);
t_chunk *get_chunk_from_ptr(void *ptr);

int validate_chunk(t_chunk *chunk);
int validate_zone(t_zone *zone);

#endif
