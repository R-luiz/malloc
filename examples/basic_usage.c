/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic_usage.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 11:50:57 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * Basic usage example for the custom malloc implementation
 * 
 * This example demonstrates:
 * - Basic malloc/free operations
 * - Different allocation sizes
 * - Memory usage patterns
 * - show_alloc_mem() functionality
 * 
 * Following NASA C guidelines:
 * - Clear, simple functions
 * - Explicit error checking
 * - Predictable behavior
 */

#include "../include/malloc.h"
#include <stdio.h>
#include <string.h>

/**
 * Demonstrate basic malloc functionality
 */
static void demo_basic_malloc(void)
{
    printf("\n=== Basic Malloc Demo ===\n");
    
    // Allocate small block
    void *ptr1 = malloc(64);
    if (ptr1) {
        printf("✓ Allocated 64 bytes at %p\n", ptr1);
        strcpy((char*)ptr1, "Hello, malloc!");
        printf("✓ Wrote data: %s\n", (char*)ptr1);
    } else {
        printf("✗ Failed to allocate 64 bytes\n");
        return;
    }
    
    // Allocate medium block
    void *ptr2 = malloc(512);
    if (ptr2) {
        printf("✓ Allocated 512 bytes at %p\n", ptr2);
        memset(ptr2, 0xAA, 512);
        printf("✓ Filled with pattern 0xAA\n");
    } else {
        printf("✗ Failed to allocate 512 bytes\n");
    }
    
    // Allocate large block
    void *ptr3 = malloc(4096);
    if (ptr3) {
        printf("✓ Allocated 4096 bytes at %p\n", ptr3);
        memset(ptr3, 0x55, 4096);
        printf("✓ Filled with pattern 0x55\n");
    } else {
        printf("✗ Failed to allocate 4096 bytes\n");
    }
    
    printf("\nMemory state after allocations:\n");
    show_alloc_mem();
    
    // Free memory
    printf("\nFreeing memory...\n");
    if (ptr1) {
        free(ptr1);
        printf("✓ Freed ptr1\n");
    }
    if (ptr2) {
        free(ptr2);
        printf("✓ Freed ptr2\n");
    }
    if (ptr3) {
        free(ptr3);
        printf("✓ Freed ptr3\n");
    }
    
    printf("\nMemory state after cleanup:\n");
    show_alloc_mem();
}

/**
 * Demonstrate realloc functionality
 */
static void demo_realloc(void)
{
    printf("\n=== Realloc Demo ===\n");
    
    // Start with small allocation
    void *ptr = malloc(100);
    if (!ptr) {
        printf("✗ Initial malloc failed\n");
        return;
    }
    
    strcpy((char*)ptr, "Initial data");
    printf("✓ Initial allocation: 100 bytes, data: %s\n", (char*)ptr);
    
    // Expand memory
    ptr = realloc(ptr, 500);
    if (ptr) {
        printf("✓ Expanded to 500 bytes, data preserved: %s\n", (char*)ptr);
        strcat((char*)ptr, " - expanded!");
        printf("✓ New data: %s\n", (char*)ptr);
    } else {
        printf("✗ Realloc expansion failed\n");
        return;
    }
    
    // Shrink memory
    ptr = realloc(ptr, 200);
    if (ptr) {
        printf("✓ Shrunk to 200 bytes, data: %s\n", (char*)ptr);
    } else {
        printf("✗ Realloc shrinking failed\n");
        return;
    }
    
    printf("\nMemory state during realloc:\n");
    show_alloc_mem();
    
    // Clean up
    free(ptr);
    printf("✓ Memory freed\n");
}

/**
 * Demonstrate error handling
 */
static void demo_error_handling(void)
{
    printf("\n=== Error Handling Demo ===\n");
    
    // Test NULL free (should be safe)
    printf("Testing free(NULL)...\n");
    free(NULL);
    printf("✓ free(NULL) completed safely\n");
    
    // Test zero size malloc
    printf("Testing malloc(0)...\n");
    void *ptr = malloc(0);
    if (ptr == NULL) {
        printf("✓ malloc(0) correctly returned NULL\n");
    } else {
        printf("⚠ malloc(0) returned %p (implementation defined)\n", ptr);
        free(ptr);
    }
    
    // Test realloc with NULL pointer (should act like malloc)
    printf("Testing realloc(NULL, 100)...\n");
    ptr = realloc(NULL, 100);
    if (ptr) {
        printf("✓ realloc(NULL, 100) worked like malloc\n");
        strcpy((char*)ptr, "realloc as malloc");
        printf("✓ Data written: %s\n", (char*)ptr);
        free(ptr);
    } else {
        printf("✗ realloc(NULL, 100) failed\n");
    }
    
    // Test realloc with size 0 (should act like free)
    ptr = malloc(50);
    if (ptr) {
        printf("Testing realloc(ptr, 0)...\n");
        ptr = realloc(ptr, 0);
        if (ptr == NULL) {
            printf("✓ realloc(ptr, 0) correctly acted like free\n");
        } else {
            printf("⚠ realloc(ptr, 0) returned %p (implementation defined)\n", ptr);
            free(ptr);
        }
    }
}

/**
 * Main demonstration function
 */
int main(void)
{
    printf("🏗️  Custom Malloc Implementation - Basic Usage Example\n");
    printf("======================================================\n");
    
    printf("\nInitial memory state:\n");
    show_alloc_mem();
    
    // Run demonstrations
    demo_basic_malloc();
    demo_realloc();
    demo_error_handling();
    
    printf("\nFinal memory state:\n");
    show_alloc_mem();
    
    printf("\n✅ Basic usage example completed successfully!\n");
    printf("\nNASA C Guidelines demonstrated:\n");
    printf("- Simple, predictable functions\n");
    printf("- Explicit error checking\n");
    printf("- Clear memory management\n");
    printf("- Comprehensive validation\n");
    
    return 0;
}**************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic_usage.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/18 11:35:30 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* Basic usage examples for the malloc library */
