# Malloc Test Suite

This directory contains comprehensive tests for the custom malloc implementation.

## Test Structure

### Individual Test Files

- **`test_malloc.c`** - Tests for `malloc()` function
  - Basic allocation functionality
  - Different allocation sizes (tiny, small, medium, large)
  - Multiple allocations
  - Edge cases and error conditions

- **`test_free.c`** - Tests for `free()` function
  - Basic free functionality
  - Free with different allocation sizes
  - Free order independence
  - Memory reuse after free

- **`test_realloc.c`** - Tests for `realloc()` function
  - Basic realloc functionality (expand/shrink)
  - Data preservation during resize
  - Realloc between different allocation types
  - Edge cases (NULL pointer, size 0, same size)

### Coordinated Test Files

- **`test_runner.c`** - Coordinates all individual tests
  - Runs all test suites with timing
  - Provides unified reporting
  - Tests `show_alloc_mem()` function
  - Basic performance benchmarks

- **`test_all.c`** - Comprehensive test suite
  - All individual function tests
  - Integration tests (function interactions)
  - Stress tests (many allocations, fragmentation)
  - Performance benchmarks
  - Memory leak detection
  - Process isolation for crash protection

## Building Tests

```bash
# Build all test executables
make -C tests all

# Or build specific tests
make -C tests test-malloc
make -C tests test-free
make -C tests test-realloc
```

## Running Tests

### Quick Start
```bash
# Run all tests
make -C tests test

# Run quick test subset
make -C tests test-quick
```

### Individual Test Suites
```bash
# Run specific function tests
make -C tests test-malloc
make -C tests test-free
make -C tests test-realloc

# Run coordinated test runner
make -C tests test-runner
```

### Specialized Tests
```bash
# Performance benchmarks
make -C tests test-perf

# Stress testing
make -C tests test-stress

# Memory leak detection
make -C tests test-leaks

# Valgrind analysis (if available)
make -C tests test-valgrind
```

### Advanced Usage
```bash
# Run specific test categories
./tests/build/test_bin/test_all malloc free
./tests/build/test_bin/test_all integration stress
./tests/build/test_bin/test_all --verbose performance

# Run with options
./tests/build/test_bin/test_all --quick
./tests/build/test_bin/test_all --help
```

## Test Categories

### 1. Basic Functionality Tests
- Verify that each function works correctly in normal conditions
- Test return values and error handling
- Validate memory can be read/written correctly

### 2. Size Variation Tests
- **Tiny allocations**: < 256 bytes
- **Small allocations**: 256 bytes - 1KB
- **Medium allocations**: 1KB - 4KB  
- **Large allocations**: > 4KB

### 3. Integration Tests
- Test interactions between malloc, free, and realloc
- Complex allocation patterns
- Mixed size allocations

### 4. Edge Case Tests
- Zero-size allocations
- Very large allocations
- NULL pointer handling
- Double free detection (if implemented)

### 5. Stress Tests
- Many small allocations
- Memory fragmentation scenarios
- Allocation/free patterns that test zone management

### 6. Performance Tests
- Allocation/deallocation speed
- Memory usage efficiency
- Realloc performance

### 7. Memory Leak Tests
- Using `show_alloc_mem()` to verify cleanup
- Partial allocation cleanup
- Complex allocation scenarios

## Test Output

### Success Indicators
- ✓ Individual test passes
- 🎉 All tests passed message
- Clean memory state shown by `show_alloc_mem()`

### Failure Indicators
- ✗ Individual test failures
- ❌ Test suite failures  
- 💥 Crashes or timeouts
- Memory leaks shown by `show_alloc_mem()`

### Example Output
```
==========================================
         MALLOC TESTS
==========================================

=== Running Basic Malloc Tests ===
✓ PASS: Basic malloc(100) should succeed
✓ PASS: Memory should be writable
✓ PASS: malloc(0) should return NULL
✓ PASS: malloc(1) should succeed

--- MALLOC TESTS SUMMARY ---
Total Passed: 15
Total Failed: 0
Success Rate: 100.0%
```

## Debugging Failed Tests

### 1. Compilation Issues
- Check include paths in Makefile
- Verify malloc library is built first
- Check for missing dependencies

### 2. Runtime Failures
- Use verbose mode: `./test_all --verbose`
- Run individual test suites to isolate issues
- Check `show_alloc_mem()` output for memory state

### 3. Crashes
- Use `gdb` with test executables
- Run with Valgrind for memory errors
- Check system limits (ulimit -v)

### 4. Performance Issues
- Profile with specific performance tests
- Check for memory leaks affecting performance
- Compare with different allocation patterns

## Adding New Tests

### 1. Individual Function Tests
Add new test functions to existing test files:
```c
int test_new_feature(void)
{
    TEST_START("New Feature Tests");
    // ... test implementation
    return passed - failed;
}
```

### 2. Integration Tests
Add to `test_all.c` integration section:
```c
// Add to test_integration()
printf("\n=== Testing new integration ===\n");
// ... test implementation
```

### 3. New Test Categories
1. Create new test file following pattern
2. Add to Makefile targets
3. Include in test_runner.c and test_all.c

## Test Configuration

Tests can be configured via:
- Command line arguments
- Preprocessor definitions
- Environment variables (if implemented)

### Preprocessor Definitions
- `TEST_MALLOC_STANDALONE` - Enable standalone test main()
- `TEST_FREE_STANDALONE` - Enable standalone test main()
- `TEST_REALLOC_STANDALONE` - Enable standalone test main()

## Requirements

- GCC compiler with C99 support
- Built malloc library (libmalloc.so)
- POSIX environment for advanced features
- Optional: Valgrind for memory analysis

## Troubleshooting

### Common Issues

1. **"Cannot open source file"** - Check include paths
2. **"Undefined reference"** - Ensure malloc library is built
3. **Tests hang** - Check for infinite loops or deadlocks
4. **Segmentation fault** - Use debugger to find memory issues
5. **Tests pass but memory leaks** - Check `show_alloc_mem()` output

### Getting Help

1. Run with verbose output: `--verbose`
2. Test individual components first
3. Check memory state with `show_alloc_mem()`
4. Use debugging tools (gdb, valgrind)
5. Review test source code for expected behavior
