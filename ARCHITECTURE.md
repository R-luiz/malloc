# Project Architecture

## Directory Structure

```
malloc/
│
├── include/
│   ├── malloc.h              # Public API declarations
│   └── malloc_internal.h     # Internal structures and function prototypes
│
├── src/
│   ├── core/
│   │   ├── globals.c         # Global variables (manager, mutex)
│   │   ├── malloc.c          # malloc() implementation
│   │   ├── free.c            # free() implementation
│   │   └── realloc.c         # realloc() implementation
│   │
│   ├── zone/
│   │   └── zone.c            # Zone management
│   │       ├── get_zone_type()
│   │       ├── get_zone_size()
│   │       ├── create_zone()
│   │       ├── add_zone_to_manager()
│   │       ├── find_or_create_zone()
│   │       └── find_zone_for_chunk()
│   │
│   ├── chunk/
│   │   └── chunk.c           # Chunk management
│   │       ├── create_chunk_in_zone()
│   │       ├── find_free_chunk()
│   │       ├── split_chunk()
│   │       ├── merge_adjacent_chunks()
│   │       ├── get_user_ptr()
│   │       └── get_chunk_from_ptr()
│   │
│   └── utils/
│       ├── show_alloc_mem.c  # Memory visualization
│       └── stats.c           # Statistics and validation
│           ├── malloc_validate_system()
│           ├── get_malloc_stats()
│           └── check_malloc_leaks()
│
├── lib/
│   └── libft/                # External libft dependency
│
├── build/                    # Build output (gitignored)
│   ├── obj/                  # Object files
│   └── bin/                  # Library output
│
├── test_minimal.c            # Test suite
├── Makefile                  # Build system
├── README.md                 # Project documentation
├── ARCHITECTURE.md           # This file
├── VERIFICATION.md           # Compliance verification
└── .gitignore                # Git ignore rules
```

## Module Responsibilities

### Core Module
**Purpose**: Implements the public API functions

**Files**:
- `globals.c`: Defines global zone manager and mutex
- `malloc.c`: Main allocation function
- `free.c`: Deallocation function
- `realloc.c`: Reallocation function

**Key Features**:
- Thread-safe operations (mutex locks)
- Edge case handling (NULL, zero size)
- Proper error handling

---

### Zone Module
**Purpose**: Manages memory zones (TINY/SMALL/LARGE)

**File**: `zone.c`

**Functions**:
1. `get_zone_type()` - Determines zone category based on size
2. `get_zone_size()` - Returns pre-allocated zone size
3. `create_zone()` - Allocates new zone via mmap
4. `add_zone_to_manager()` - Links zone to global manager
5. `find_or_create_zone()` - Finds suitable zone or creates new one
6. `find_zone_for_chunk()` - Locates zone containing a chunk

**Key Features**:
- Pre-allocated zones for TINY/SMALL
- Direct mmap for LARGE
- Zone reuse for efficiency

---

### Chunk Module
**Purpose**: Manages individual allocations within zones

**File**: `chunk.c`

**Functions**:
1. `create_chunk_in_zone()` - Creates new chunk in zone
2. `find_free_chunk()` - Searches for available free chunk
3. `split_chunk()` - Divides oversized chunk
4. `merge_adjacent_chunks()` - Coalesces free chunks
5. `get_user_ptr()` - Converts chunk to user pointer
6. `get_chunk_from_ptr()` - Converts user pointer to chunk

**Key Features**:
- First-fit allocation strategy
- Chunk splitting to reduce waste
- Adjacent chunk merging on free

---

### Utils Module
**Purpose**: Provides diagnostic and monitoring functions

**Files**:
- `show_alloc_mem.c`: Memory visualization
- `stats.c`: Statistics and validation

**Functions**:
1. `show_alloc_mem()` - Displays all allocations by zone
2. `get_malloc_stats()` - Retrieves allocation statistics
3. `check_malloc_leaks()` - Counts unreleased allocations
4. `malloc_validate_system()` - System integrity check

**Key Features**:
- Non-intrusive monitoring
- Thread-safe statistics
- Debugging support

---

## Data Flow

### malloc() Flow

```
User calls malloc(size)
         ↓
    malloc.c: malloc()
         ↓
    zone.c: get_zone_type(size) → TINY/SMALL/LARGE
         ↓
    zone.c: find_or_create_zone(type, size)
         ↓
    ┌────────────────┐
    │ Zone exists?   │
    └────┬───────┬───┘
         │       │
       YES       NO
         │       │
         │       └→ zone.c: create_zone() → mmap()
         │                     ↓
         └──────────→ chunk.c: find_free_chunk()
                               ↓
                     ┌─────────────────┐
                     │ Free chunk?     │
                     └────┬────────┬───┘
                          │        │
                        YES       NO
                          │        │
                          │        └→ chunk.c: create_chunk_in_zone()
                          │
                          └→ chunk.c: split_chunk() (if needed)
                                      ↓
                            chunk.c: get_user_ptr()
                                      ↓
                            Return pointer to user
```

### free() Flow

```
User calls free(ptr)
         ↓
    free.c: free()
         ↓
    chunk.c: get_chunk_from_ptr(ptr)
         ↓
    Set chunk->is_free = 1
         ↓
    chunk.c: merge_adjacent_chunks()
         ↓
    Release mutex
```

### realloc() Flow

```
User calls realloc(ptr, size)
         ↓
    realloc.c: realloc()
         ↓
    ┌───────────────────┐
    │ ptr == NULL?      │
    └────┬──────────────┘
         │
       YES → malloc(size)
         │
       NO
         ↓
    ┌───────────────────┐
    │ size == 0?        │
    └────┬──────────────┘
         │
       YES → free(ptr)
         │
       NO
         ↓
    chunk.c: get_chunk_from_ptr(ptr)
         ↓
    ┌───────────────────────────┐
    │ Current size >= new size? │
    └────┬──────────────────┬───┘
         │                  │
       YES                 NO
         │                  │
         │                  └→ malloc(new_size)
         │                           ↓
         │                     memcpy(old → new)
         │                           ↓
         │                     free(old)
         │
         └→ chunk.c: split_chunk()
                    ↓
            Return same pointer
```

---

## Dependencies

### External Dependencies
- **libft**: Custom C library
- **pthread**: Thread synchronization
- **sys/mman.h**: mmap/munmap
- **unistd.h**: Page size functions

### Internal Dependencies
```
malloc.h (Public API)
    ↑
    │
malloc_internal.h (Internal API)
    ↑
    ├─── core/ (malloc, free, realloc)
    ├─── zone/ (zone management)
    ├─── chunk/ (chunk management)
    └─── utils/ (diagnostics)
```

**Dependency Rules**:
1. All modules include `malloc_internal.h`
2. No circular dependencies
3. Modules don't include each other
4. Clean separation of concerns

---

## Build Process

### Compilation Steps

1. **Build libft**
   ```bash
   make -C lib/
   ```

2. **Compile object files** (per module)
   ```bash
   gcc -c src/core/*.c -o build/obj/core/*.o
   gcc -c src/zone/*.c -o build/obj/zone/*.o
   gcc -c src/chunk/*.c -o build/obj/chunk/*.o
   gcc -c src/utils/*.c -o build/obj/utils/*.o
   ```

3. **Link shared library**
   ```bash
   gcc -shared *.o -lft -lpthread -o libft_malloc_$(HOSTTYPE).so
   ```

4. **Create symlinks**
   ```bash
   ln -s libft_malloc_$(HOSTTYPE).so libft_malloc.so
   ```

---

## Testing Strategy

### Unit Testing
Each module can be tested independently:
- Zone creation and management
- Chunk allocation and merging
- Statistics accuracy

### Integration Testing
Test file: `test_minimal.c`

**Test Cases**:
1. TINY allocations
2. SMALL allocations
3. LARGE allocations
4. Mixed sizes
5. Free and reuse
6. Realloc functionality
7. Statistics tracking
8. Zone efficiency

---

## Coding Standards

### NASA C Standards Compliance

✅ **Function Length**: All functions < 60 lines
✅ **Loop Bounds**: All loops have iteration limits
✅ **Single Responsibility**: One purpose per function
✅ **Error Handling**: Explicit checks and returns
✅ **No Magic Numbers**: All constants named

### Naming Conventions

- **Types**: `t_name` (e.g., `t_zone`, `t_chunk`)
- **Functions**: `snake_case` (e.g., `find_free_chunk`)
- **Constants**: `UPPER_CASE` (e.g., `TINY_MAX`)
- **Globals**: `g_name` (e.g., `g_manager`, `g_mutex`)

### Code Style

- **Indentation**: Tabs
- **Line Length**: < 80 characters
- **Comments**: Minimal, only for complex logic
- **Braces**: K&R style

---

## Performance Characteristics

### Time Complexity

| Operation | TINY/SMALL | LARGE |
|-----------|------------|-------|
| malloc()  | O(n)*      | O(1)  |
| free()    | O(1)       | O(1)  |
| realloc() | O(n)*      | O(n)  |

*n is bounded at 10,000 chunks

### Space Complexity

- **Zone overhead**: 64 bytes per zone
- **Chunk overhead**: 32 bytes per allocation
- **Alignment**: 16-byte aligned allocations

### System Calls

- **mmap()**: Only on zone creation or LARGE alloc
- **TINY zone**: Reused for ~400 allocations (128 bytes each)
- **SMALL zone**: Reused for ~400 allocations (1024 bytes each)

---

## Future Enhancements

Possible improvements (not in current scope):
- Heap defragmentation
- Corruption detection (magic numbers, checksums)
- Better statistics (fragmentation ratio, peak usage)
- Memory pool pre-allocation
- Debug mode with additional checks

---

## Maintenance

### Adding New Features

1. Identify appropriate module
2. Add function prototype to `malloc_internal.h`
3. Implement in module file
4. Update Makefile if adding new file
5. Update documentation

### Debugging

- Use `show_alloc_mem()` to visualize state
- Check `get_malloc_stats()` for metrics
- Use `check_malloc_leaks()` before exit
- Compile with `-g3` for debugging symbols

---

## Conclusion

This architecture provides:
- **Clean separation** of responsibilities
- **Easy maintenance** with modular design
- **Scalability** for future enhancements
- **Testability** at module level
- **Documentation** for understanding

The modular design makes it easy to understand, modify, and extend while maintaining NASA C coding standards and project requirements.
