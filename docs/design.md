# Design Documentation

## Overview

This malloc implementation follows a zone-based allocation strategy with three different allocation sizes:

- **TINY**: Small allocations (< 256 bytes)
- **SMALL**: Medium allocations (256 bytes - 1KB)  
- **LARGE**: Large allocations (> 1KB)

## Architecture

### Zone Management

Each zone is a large memory region allocated with `mmap()` that contains multiple blocks for individual malloc requests.

### Block Management

Individual allocations within zones are managed using a linked list of blocks with metadata for each allocation.

### Thread Safety

All public functions are protected with mutexes to ensure thread safety.
