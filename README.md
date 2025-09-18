# Malloc Project

A custom implementation of malloc, free, and realloc functions following NASA C coding standards and clean architecture principles.

## 🏗️ Architecture

This project follows NASA C coding guidelines and clean architecture principles with clear separation of concerns:

### Directory Structure
```
├── include/           # Public API headers
│   ├── malloc.h      # Main public interface
│   └── types.h       # Public type definitions
├── src/              # Source code (NASA modular design)
│   ├── core/         # Core malloc functionality
│   │   ├── malloc.c  # Memory allocation
│   │   ├── free.c    # Memory deallocation
│   │   └── realloc.c # Memory reallocation
│   ├── utils/        # Utility functions
│   │   ├── show_alloc_mem.c # Debug output
│   │   ├── memory_utils.c   # Memory utilities
│   │   └── debug.c          # Debug functions
│   └── internal/     # Internal headers
│       ├── malloc_internal.h # Internal structures
│       └── zone_manager.h    # Zone management
├── tests/            # Comprehensive test suite
│   ├── test_malloc.c    # malloc() tests
│   ├── test_free.c      # free() tests
│   ├── test_realloc.c   # realloc() tests
│   ├── test_runner.c    # Coordinated testing
│   ├── test_all.c       # Comprehensive tests
│   └── test_common.h    # Shared test utilities
├── examples/         # Usage examples
│   └── basic_usage.c    # Basic demonstration
├── docs/             # Documentation
│   ├── api.md           # API documentation
│   └── design.md        # Design documentation
├── lib/              # External libraries (libft)
└── build/            # Build artifacts
    ├── obj/             # Object files
    └── bin/             # Executables and libraries
```

### NASA C Guidelines Compliance

✅ **Modularity**: Clear separation between core, utils, and internal components  
✅ **Simplicity**: Functions under 60 lines, clear interfaces  
✅ **Predictability**: Deterministic behavior, explicit error handling  
✅ **Verifiability**: Comprehensive test suite with individual component testing  
✅ **Maintainability**: Clear documentation, consistent coding style  

## 🚀 Building

### Prerequisites
- GCC with C99 support
- Make
- POSIX environment (Linux/macOS)

### Quick Start
```bash
# Build the library
make

# Run comprehensive tests
make test

# Run quick validation
make test-quick

# Get help
make help
```

### Advanced Building
```bash
# Build specific components
make all                    # Build library only
make test-build            # Build test suite
make examples              # Build examples

# Different test levels
make test-malloc           # Test malloc() only
make test-free             # Test free() only
make test-realloc          # Test realloc() only
make test-performance      # Performance benchmarks
make test-stress           # Stress testing
make test-leaks            # Memory leak detection
```

## 🧪 Testing

### Test Architecture (NASA Principles)

The test suite follows NASA guidelines for safety-critical software:

1. **Component Testing**: Each function tested independently
2. **Integration Testing**: Function interactions validated
3. **Stress Testing**: Edge cases and limits
4. **Performance Testing**: Benchmarks and efficiency
5. **Error Testing**: Failure modes and recovery

### Test Execution
```bash
# Quick development tests
make test-quick

# Full validation suite  
make test

# Individual function testing
make test-malloc
make test-free
make test-realloc

# Advanced testing
make test-performance      # Speed benchmarks
make test-stress          # Fragmentation, limits
make test-leaks           # Memory leak detection
make test-valgrind        # Valgrind analysis
```

### Test Output Example
```
🧪 Running comprehensive test suite...
======================================

==========================================
         MALLOC TESTS
==========================================

=== Running Basic Malloc Tests ===
✓ PASS: Basic malloc(100) should succeed
✓ PASS: Memory should be writable
✓ PASS: malloc(0) should return NULL

--- MALLOC TESTS SUMMARY ---
Total Passed: 15
Total Failed: 0
Success Rate: 100.0%
```

## 📋 Usage

### Basic API
```c
#include "malloc.h"

// Allocate memory
void *ptr = malloc(size);

// Free memory  
free(ptr);

// Reallocate memory
ptr = realloc(ptr, new_size);

// Show memory status (debug)
show_alloc_mem();
```

### Examples
```bash
# Run basic usage example
make -C examples run-basic

# View example source
cat examples/basic_usage.c
```

## 🔧 Configuration

### Build Configuration
```bash
# Debug build (default)
make CFLAGS="-Wall -Wextra -Werror -g3"

# Release build  
make CFLAGS="-Wall -Wextra -Werror -O3 -DNDEBUG"

# Custom installation
make install PREFIX=/custom/path
```

### Host Platform Detection
The library automatically detects the platform:
- **Linux**: Uses `sysconf(_SC_PAGESIZE)`  
- **macOS**: Uses `getpagesize()`
- **Others**: Falls back to 4096-byte pages

## 📖 Documentation

- **[API Documentation](docs/api.md)** - Function interfaces and usage
- **[Design Documentation](docs/design.md)** - Architecture and implementation
- **[Test Documentation](tests/README.md)** - Testing strategy and execution

## 🛠️ Development

### Adding New Features
1. Follow NASA C guidelines (functions < 60 lines)
2. Add to appropriate module (core/utils/internal)
3. Write comprehensive tests
4. Update documentation
5. Validate with test suite

### Debugging
```bash
# Build with debug symbols
make clean && make

# Run with debugging
gdb build/test_bin/test_all

# Memory analysis
make test-valgrind

# Performance profiling  
make test-performance
```

### Code Style
- NASA C coding standards compliance
- Clear, descriptive function names
- Explicit error checking
- Comprehensive documentation
- Modular design

## 🔍 Performance

### Allocation Strategy
- **Zone-based allocation**: Minimizes mmap() calls
- **Size categorization**: Tiny, small, medium, large
- **Memory reuse**: Efficient free block management
- **Alignment**: Proper memory alignment for performance

### Benchmarks
Run performance tests to see current metrics:
```bash
make test-performance
```

## 🐛 Troubleshooting

### Common Issues

1. **Build Failures**
   ```bash
   # Check dependencies
   make clean && make
   
   # Verify libft is built
   make -C lib clean && make -C lib
   ```

2. **Test Failures**
   ```bash
   # Run individual tests to isolate
   make test-malloc
   
   # Check memory state
   make test-leaks
   ```

3. **Runtime Issues**
   ```bash
   # Use debugging tools
   make test-valgrind
   
   # Check library path
   export LD_LIBRARY_PATH=./build/bin:$LD_LIBRARY_PATH
   ```

### Getting Help
1. Check documentation in `docs/`
2. Run `make help` for available targets
3. Examine test output for specific failures
4. Use debugging tools (gdb, valgrind)

## 📄 License

This project follows 42 School guidelines and educational use policies.

## 🏆 Quality Assurance

✅ **NASA C Standards**: Compliant with JPL coding standards  
✅ **Memory Safety**: Comprehensive testing for leaks and corruption  
✅ **Performance**: Optimized allocation strategies  
✅ **Portability**: Cross-platform compatibility (Linux/macOS)  
✅ **Maintainability**: Clear architecture and documentation  
✅ **Testability**: 100% function coverage with multiple test levels
