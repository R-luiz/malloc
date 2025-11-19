# Custom Malloc Implementation

A clean, efficient zone-based malloc implementation following NASA C coding standards.

## Features

- **Zone-based allocation** (TINY/SMALL/LARGE)
- **Efficient memory reuse** with pre-allocated zones
- **Thread-safe** operations
- **Memory alignment** (16-byte aligned)
- **Chunk splitting and merging** for optimal space usage
- **NASA C compliant** (functions < 60 lines, bounded loops)

## Architecture

```
malloc/
├── include/
│   ├── malloc.h              # Public API
│   └── malloc_internal.h     # Internal structures and declarations
├── src/
│   ├── core/
│   │   ├── globals.c         # Global variables
│   │   ├── malloc.c          # malloc() implementation
│   │   ├── free.c            # free() implementation
│   │   └── realloc.c         # realloc() implementation
│   ├── zone/
│   │   └── zone.c            # Zone management
│   ├── chunk/
│   │   └── chunk.c           # Chunk management
│   └── utils/
│       ├── show_alloc_mem.c  # Memory visualization
│       └── stats.c           # Statistics and validation
├── lib/
│   └── ...                   # libft dependency
├── Makefile
└── test_minimal.c            # Test suite
```

## Zone Types

| Type  | Size Range      | Pre-allocated Size |
|-------|----------------|--------------------|
| TINY  | ≤ 128 bytes    | 64 KB (16 pages)   |
| SMALL | 129-1024 bytes | 425 KB (104 pages) |
| LARGE | > 1024 bytes   | Custom per alloc   |

## Building

```bash
# Build the library
make

# Clean build
make fclean && make

# Show help
make help
```

## Usage

### Compile your program with the library

```bash
gcc -o your_program your_program.c -L./build/bin -lft_malloc -lpthread
export LD_LIBRARY_PATH=./build/bin:$LD_LIBRARY_PATH
./your_program
```

### Basic Usage

```c
#include "malloc.h"

int main(void)
{
    void *ptr = malloc(100);

    show_alloc_mem();

    free(ptr);
    return 0;
}
```

## API

### Core Functions

```c
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
```

### Utility Functions

```c
void show_alloc_mem(void);
int get_malloc_stats(t_malloc_stats *stats);
int check_malloc_leaks(void);
int malloc_validate_system(void);
```

## Memory Layout

### Zone Structure

```
┌─────────────────────────────────────┐
│ Zone Header (t_zone)                │
├─────────────────────────────────────┤
│ Chunk Header | User Data            │
│ Chunk Header | User Data            │
│ Chunk Header | User Data            │
│ ...                                 │
└─────────────────────────────────────┘
```

### Allocation Flow

1. Determine zone type based on size
2. Find existing zone with free space
3. Search for free chunk or create new one
4. Split chunk if oversized
5. Return pointer to user data

## NASA C Standards Compliance

✅ All functions under 60 lines
✅ Bounded loops (no infinite loops)
✅ Single responsibility per function
✅ Explicit error handling
✅ No magic numbers
✅ Clear structure and naming

## File Organization

### Core Module
Implements the main API functions (malloc, free, realloc) with thread safety.

### Zone Module
Manages memory zones, including creation, type determination, and zone search.

### Chunk Module
Handles individual allocations within zones, including splitting and merging.

### Utils Module
Provides diagnostic and monitoring functions.

## Performance

- **TINY/SMALL allocations**: O(n) free chunk search (bounded at 10,000)
- **LARGE allocations**: O(1) direct mmap
- **Zone reuse**: Minimizes expensive mmap() syscalls
- **Memory overhead**: ~32 bytes per allocation

## Thread Safety

All public functions use pthread_mutex for protection:
- malloc()
- free()
- realloc()
- show_alloc_mem()
- get_malloc_stats()
- check_malloc_leaks()

## Testing

```bash
# Run test suite
gcc -o test test_minimal.c -L./build/bin -lft_malloc -lpthread
LD_LIBRARY_PATH=./build/bin ./test
```

Tests cover:
- TINY allocations
- SMALL allocations
- LARGE allocations
- Mixed allocations
- Free and zone reuse
- Realloc functionality
- Statistics tracking
- Zone efficiency

## Project Status

✅ Zone-based allocation implemented
✅ Thread-safe operations
✅ Proper show_alloc_mem() output
✅ NASA C standards compliant
✅ Clean architecture
✅ Comprehensive testing

**Status: Production Ready**
