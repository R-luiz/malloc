# API Documentation

## Public Functions

### malloc

```c
void *malloc(size_t size);
```

Allocates `size` bytes and returns a pointer to the allocated memory.

- **Parameters**: `size` - Number of bytes to allocate
- **Returns**: Pointer to allocated memory, or NULL on failure
- **Thread Safety**: Yes

### free

```c
void free(void *ptr);
```

Frees the memory space pointed to by `ptr`.

- **Parameters**: `ptr` - Pointer previously returned by malloc
- **Returns**: None
- **Thread Safety**: Yes

### realloc

```c
void *realloc(void *ptr, size_t size);
```

Changes the size of the memory block pointed to by `ptr`.

- **Parameters**: 
  - `ptr` - Pointer to previously allocated memory
  - `size` - New size in bytes
- **Returns**: Pointer to reallocated memory, or NULL on failure
- **Thread Safety**: Yes

### show_alloc_mem

```c
void show_alloc_mem(void);
```

Displays current memory allocation status.

- **Parameters**: None
- **Returns**: None
- **Thread Safety**: Yes
