# ft_malloc

A production-ready, thread-safe memory allocator implementing zone-based allocation strategies with comprehensive security features and diagnostic tools.

## Overview

This project reimplements the standard C library functions `malloc()`, `free()`, and `realloc()` from scratch using only system calls. The implementation follows a zone-based allocation strategy that balances performance with memory efficiency, categorizing allocations into TINY, SMALL, and LARGE zones.

**Key Features:**
- Zone-based allocation (TINY/SMALL/LARGE)
- Thread-safe operations with mutex protection
- 16-byte memory alignment
- Chunk coalescing and splitting
- Double-free and invalid pointer protection
- Memory cleanup and leak detection
- Zero system library dependencies for core operations

## Technical Specifications

### Memory Zones

| Zone Type | Size Range | Pre-allocated Size | Allocations/Zone |
|-----------|------------|-------------------|------------------|
| TINY      | 1-128 bytes | 64 KB (16 pages) | ~372 |
| SMALL     | 129-1024 bytes | 425 KB (104 pages) | ~397 |
| LARGE     | >1024 bytes | Custom (mmap) | 1 per zone |

**Design Rationale:**
- TINY/SMALL zones are pre-allocated and reused to minimize expensive `mmap()` syscalls
- LARGE allocations receive dedicated zones and are immediately freed back to the OS
- Zone sizes are multiples of page size for optimal memory mapping
- Each zone can accommodate 100+ allocations (subject requirement satisfied)

### Architecture

```
malloc/
├── include/
│   ├── malloc.h              Public API
│   └── malloc_internal.h     Internal structures
├── src/
│   ├── core/                 Core allocation functions
│   │   ├── globals.c         Global state management
│   │   ├── malloc.c          Memory allocation
│   │   ├── free.c            Memory deallocation
│   │   └── realloc.c         Memory reallocation
│   ├── zone/                 Zone management
│   │   └── zone.c            Zone creation and lifecycle
│   ├── chunk/                Chunk management
│   │   └── chunk.c           Chunk operations and merging
│   └── utils/                Utilities and diagnostics
│       ├── show_alloc_mem.c  Memory visualization
│       ├── stats.c           Statistics tracking
│       ├── cleanup.c         Zone cleanup functions
│       ├── output.c          Write-based output
│       └── memory.c          Memory operations
├── lib/                      libft dependency
├── Makefile                  Build system
└── tests/                    Test programs
```

## API Reference

### Core Functions

#### `void *malloc(size_t size)`
Allocates `size` bytes of memory and returns a pointer to the allocated space.

**Behavior:**
- Returns `NULL` if `size` is 0 or allocation fails
- Determines zone type based on size (TINY/SMALL/LARGE)
- Searches for free chunk in existing zones
- Creates new zone if necessary
- Splits oversized chunks when beneficial
- Returns 16-byte aligned pointer

**Time Complexity:**
- TINY/SMALL: O(n) where n ≤ 10,000 (bounded)
- LARGE: O(1) direct mmap

#### `void free(void *ptr)`
Deallocates the memory allocation pointed to by `ptr`.

**Behavior:**
- No-op if `ptr` is `NULL`
- Validates pointer before freeing:
  - Checks 16-byte alignment
  - Verifies magic number (CHUNK_MAGIC_ALLOCATED)
  - Confirms pointer is within valid zone
  - Prevents double-free attempts
  - Rejects stack/invalid pointers
- Marks chunk as free and merges with adjacent free chunks
- Munmaps LARGE zones immediately
- TINY/SMALL zones remain for reuse (arena optimization)

**Time Complexity:** O(1)

#### `void *realloc(void *ptr, size_t size)`
Changes the size of the memory block pointed to by `ptr` to `size` bytes.

**Behavior:**
- If `ptr` is `NULL`, equivalent to `malloc(size)`
- If `size` is 0, equivalent to `free(ptr)` and returns `NULL`
- If current size ≥ new size, shrinks in place and returns same pointer
- If growing: allocates new block, copies data, frees old block
- Validates pointer before reallocation

**Time Complexity:**
- Shrink: O(1)
- Grow: O(n) where n is data size to copy

#### `void show_alloc_mem(void)`
Displays all allocated memory zones and chunks in ascending address order.

**Output Format:**
```
TINY : 0xA0000
0xA0020 - 0xA004A : 42 bytes
0xA006A - 0xA00BE : 84 bytes
SMALL : 0xAD000
0xAD020 - 0xADEAD : 3725 bytes
LARGE : 0xB0000
0xB0020 - 0xBBEEF : 48847 bytes
Total : 52698 bytes
```

### Diagnostic Functions

#### `int malloc_cleanup(void)`
Frees empty zones back to the operating system while preserving zones with active allocations.

**Returns:** Number of zones freed

**Use Case:** Reduce memory footprint during low-usage periods

#### `void malloc_destroy(void)`
Complete memory cleanup - zeros and frees ALL zones.

**Warning:** Invalidates all malloc'd pointers. Only call before program exit.

**Use Case:** Security (zero sensitive data), leak detection, testing

#### `int get_malloc_stats(t_malloc_stats *stats)`
Retrieves comprehensive allocation statistics.

**Statistics Provided:**
- Total bytes allocated
- TINY/SMALL/LARGE allocation counts
- Active and total zone counts

#### `int check_malloc_leaks(void)`
Scans all zones for unreleased allocations.

**Returns:** Number of leaked allocations

## Build & Installation

### Requirements

- GCC compiler with C99 support
- pthread library
- GNU Make
- libft (included in `lib/` directory)

### Building

```bash
# Full build
make

# Clean build artifacts
make clean

# Complete rebuild
make fclean && make

# Build with debug symbols (default)
make CFLAGS="-Wall -Wextra -Werror -fPIC -g3 -std=c99"
```

### Output

The build produces:
- `build/bin/libft_malloc_$HOSTTYPE.so` - Main shared library
- `build/bin/libft_malloc.so` - Symbolic link
- `libft_malloc.so` - Root-level symbolic link

### Using the Library

```bash
# Compile your program
gcc -o your_program your_program.c -L./build/bin -lft_malloc -lpthread

# Set library path
export LD_LIBRARY_PATH=./build/bin:$LD_LIBRARY_PATH

# Run
./your_program
```

## Usage Examples

### Basic Usage

```c
#include "malloc.h"

int main(void)
{
    // Allocate memory
    char *str = malloc(100);
    if (!str)
        return 1;

    // Use memory
    strcpy(str, "Hello, malloc!");

    // Reallocate if needed
    str = realloc(str, 200);

    // Visualize memory state
    show_alloc_mem();

    // Free memory
    free(str);

    return 0;
}
```

### Memory Cleanup

```c
// During runtime - free empty zones
int freed_zones = malloc_cleanup();

// Before exit - complete cleanup
malloc_destroy();  // Zeros and frees all zones
```

### Leak Detection

```c
// ... program logic ...

// Check for leaks before exit
int leaks = check_malloc_leaks();
if (leaks > 0) {
    printf("Warning: %d leaks detected\n", leaks);
}
```

### Statistics

```c
t_malloc_stats stats;
if (get_malloc_stats(&stats) == 0) {
    printf("Allocated: %zu bytes\n", stats.bytes_allocated);
    printf("TINY: %u, SMALL: %u, LARGE: %u\n",
           stats.allocs_tiny, stats.allocs_small, stats.allocs_large);
}
```

## Testing

### Comprehensive Test Suite

```bash
# Build test
gcc -o test_complete test_complete.c -L./build/bin -lft_malloc -lpthread

# Run automated tests
LD_LIBRARY_PATH=./build/bin ./test_complete
```

**Tests 20 scenarios:**
- Basic allocation (TINY/SMALL/LARGE)
- Edge cases (malloc(0), free(NULL))
- Realloc operations (grow, shrink, NULL, 0)
- Alignment verification
- Security (double-free, invalid pointer)
- Stress testing (100+ allocations per zone)
- Fragmentation handling
- Memory leak detection

### Interactive Testing

```bash
# Build interactive test
gcc -o interactive_test interactive_test_clean.c -L./build/bin -lft_malloc -lpthread

# Run interactive mode
LD_LIBRARY_PATH=./build/bin ./interactive_test
```

**Features:**
- Manual allocation/deallocation
- Real-time memory visualization
- Statistics monitoring
- Leak detection
- Stress testing capabilities

## Implementation Details

### Data Structures

#### Zone Structure
```c
typedef struct s_zone {
    uint32_t magic;          // ZONE_MAGIC (0xCAFEBABE)
    t_zone_type type;        // TINY, SMALL, or LARGE
    size_t total_size;       // Total zone size
    size_t used_size;        // Currently used bytes
    void *start;             // Zone start address
    void *end;               // Zone end address
    t_chunk *chunks;         // Linked list of chunks
    struct s_zone *next;     // Next zone in list
    size_t chunk_count;      // Number of chunks (bounded)
} t_zone;
```

#### Chunk Structure
```c
typedef struct s_chunk {
    uint32_t magic;          // ALLOCATED (0xDEADBEEF) or FREE (0xFEEDFACE)
    size_t size;             // User-requested size
    int is_free;             // Free flag
    struct s_chunk *next;    // Next chunk
    struct s_chunk *prev;    // Previous chunk
    t_zone *zone;            // Parent zone (O(1) lookup)
} t_chunk;
```

### Allocation Strategy

1. **Size Classification:**
   - TINY: ≤128 bytes → reusable 64KB zones
   - SMALL: 129-1024 bytes → reusable 425KB zones
   - LARGE: >1024 bytes → dedicated mmap zones

2. **Zone Selection:**
   - Search existing zones for free space
   - Create new zone if no suitable space found
   - LARGE allocations always get new zone

3. **Chunk Management:**
   - First-fit algorithm with bounded search (10,000 chunks max)
   - Split chunks when remainder ≥ MIN_SPLIT_SIZE (32 bytes)
   - Merge adjacent free chunks on deallocation

4. **Memory Alignment:**
   - All allocations 16-byte aligned
   - Chunk headers aligned to 16 bytes
   - Zone headers aligned to 16 bytes

### Security Features

#### Double-Free Protection
```c
// In free():
if (chunk->is_free)        // Already freed
    return;                // Silent ignore
```

#### Invalid Pointer Detection
```c
// Validation checks:
- Alignment: (ptr % 16 == 0)
- Magic number: chunk->magic == CHUNK_MAGIC_ALLOCATED
- Zone bounds: chunk >= zone->start && chunk < zone->end
- Exact match: ptr == (chunk + CHUNK_HEADER_SIZE)
```

#### Magic Numbers
- Zone: `0xCAFEBABE` - Identifies valid zones
- Allocated: `0xDEADBEEF` - Marks active allocations
- Free: `0xFEEDFACE` - Marks freed chunks

### Thread Safety

All public functions are protected by a global mutex:
- `malloc()`, `free()`, `realloc()`
- `show_alloc_mem()`, `get_malloc_stats()`, `check_malloc_leaks()`
- `malloc_cleanup()`, `malloc_destroy()`

**Mutex Type:** `pthread_mutex_t` with static initialization

**Locking Strategy:** Coarse-grained locking for simplicity and correctness

## Standards Compliance

### Subject Requirements

✅ **Mandatory Functions:**
- `malloc()`, `free()`, `realloc()` fully implemented
- `show_alloc_mem()` with correct output format

✅ **Library Naming:**
- `libft_malloc_$HOSTTYPE.so` with automatic HOSTTYPE detection
- Symbolic link `libft_malloc.so` created

✅ **Memory Management:**
- Uses only `mmap(2)` and `munmap(2)` for OS memory
- Pre-allocates TINY/SMALL zones to minimize syscalls
- Zone sizes are multiples of page size
- Each zone holds 100+ allocations

✅ **Allowed Functions:**
- `mmap(2)`, `munmap(2)` - Memory mapping
- `write(2)` - Output (for show_alloc_mem)
- `getpagesize()` / `sysconf(_SC_PAGESIZE)` - Page size
- `pthread_*` - Thread safety
- **No forbidden functions:** No printf, stdio, stdlib dependencies

✅ **Global Variables:**
- `g_manager` - Zone manager (1 variable)
- `g_mutex` - Thread safety mutex (1 variable)
- Total: 2 (subject allows 2)

### NASA C Coding Standards

✅ **Function Length:**
- All functions < 60 lines
- Longest: `cleanup_empty_zones_of_type()` at 35 lines

✅ **Loop Bounds:**
- All loops bounded with MAX_* constants
- No infinite loops
- Typical bounds: MAX_ZONES_PER_TYPE (1000), MAX_CHUNKS_PER_ZONE (10000)

✅ **Single Responsibility:**
- Each function has one clear purpose
- Complex operations broken into helper functions

✅ **Error Handling:**
- Explicit NULL checks
- Validation before operations
- Silent failures where appropriate (e.g., free(NULL))

✅ **No Magic Numbers:**
- All constants defined with meaningful names
- `TINY_MAX`, `SMALL_MAX`, `MIN_SPLIT_SIZE`, etc.

✅ **No Recursion:**
- All algorithms iterative
- Stack usage bounded

## Performance Characteristics

### Time Complexity

| Operation | TINY/SMALL | LARGE |
|-----------|------------|-------|
| malloc()  | O(n)*      | O(1)  |
| free()    | O(1)       | O(1)  |
| realloc() | O(m)**     | O(m)  |

*n bounded at 10,000 chunks per zone
**m is size of data to copy

### Space Overhead

- Zone header: 64 bytes
- Chunk header: 32 bytes per allocation
- Total per allocation: 32 bytes + user size (aligned to 16)
- Overhead percentage: ~3% for 1KB allocations, <1% for larger

### System Call Efficiency

**TINY Zone (64KB):**
- Capacity: ~372 allocations of 128 bytes
- Syscalls: 1 mmap → 372 mallocs
- Efficiency: 372:1 ratio

**SMALL Zone (425KB):**
- Capacity: ~397 allocations of 1024 bytes
- Syscalls: 1 mmap → 397 mallocs
- Efficiency: 397:1 ratio

**LARGE Allocations:**
- Syscalls: 1 mmap per allocation
- Immediately munmapped on free (no lingering memory)

## Project Metrics

**Implementation Size:**
- Source files: 11
- Header files: 2
- Total lines of code: ~818
- Functions: 30
- Average function length: ~27 lines

**Module Distribution:**
- Core (4 files): malloc, free, realloc, globals
- Zone (1 file): zone lifecycle management
- Chunk (1 file): chunk operations
- Utils (5 files): diagnostics, output, memory ops

**Code Quality:**
- Zero warnings with `-Wall -Wextra -Werror`
- All functions < 60 lines
- Consistent coding style
- Minimal comments (self-documenting code)

## Known Limitations & Design Decisions

### TINY/SMALL Zones Never Freed During Runtime

**By Design:** Subject explicitly requires "limit the number of calls to munmap()" and "pre-allocate zones". TINY/SMALL zones are kept for reuse to maximize performance.

**Rationale:**
- Matches behavior of production allocators (glibc malloc, jemalloc)
- Optimizes for common malloc/free patterns
- Reduces expensive syscalls

**Solution:** Use `malloc_cleanup()` to explicitly free empty zones when needed.

### O(n) Search for Free Chunks

**Bounded:** n ≤ 10,000 chunks per zone (NASA C requirement)

**Acceptable:** For TINY/SMALL allocations, the search is typically very fast as zones are designed to fit common allocation patterns.

**Alternative Considered:** Free list data structure (not implemented for simplicity)

### Coarse-Grained Locking

**Single Global Mutex:** All operations lock the entire allocator

**Rationale:**
- Simpler implementation
- Correct by construction
- Sufficient for most use cases

**Alternative:** Per-zone locks or lock-free data structures (overkill for project scope)

## Debugging

### Memory Visualization

```bash
# Compile with debug symbols (default)
make

# Run with memory visualization
./your_program
# In code: show_alloc_mem()
```

### Leak Detection

```bash
# Use built-in leak checker
int leaks = check_malloc_leaks();

# Or use external tools
valgrind --leak-check=full ./your_program
```

### Statistics Monitoring

```c
t_malloc_stats stats;
get_malloc_stats(&stats);
// Inspect stats structure
```

## License & Credits

This project is an educational implementation for the 42 school curriculum. It demonstrates:
- Systems programming with direct syscall usage
- Memory management algorithms
- Thread synchronization
- C coding standards adherence
- Production-quality engineering practices

**Author:** [Your Name]
**School:** 42
**Project:** ft_malloc
**Status:** ✅ Production Ready
