/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_common.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rluiz <rluiz@student.42lehavre.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/18 11:17:45 by rluiz             #+#    #+#             */
/*   Updated: 2025/09/19 10:44:58 by rluiz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEST_COMMON_H
# define TEST_COMMON_H

# include "../include/malloc.h"
# include <stdio.h>
# include <string.h>
# include <stdint.h>
# include <stdlib.h>
# include <unistd.h>
# include <time.h>

/* Test result structure */
typedef struct {
    int passed;
    int failed;
    const char *current_test;
} test_result_t;

/* Common test macros */
# define TEST_ASSERT(condition, message) do { \
    if (condition) { \
        printf("✓ PASS: %s\n", message); \
        return 1; \
    } else { \
        printf("✗ FAIL: %s\n", message); \
        return 0; \
    } \
} while(0)

# define TEST_START(name) printf("\n=== Running %s ===\n", name)

# define TEST_EXPECT(condition, message) do { \
    if (condition) { \
        printf("✓ PASS: %s\n", message); \
    } else { \
        printf("✗ FAIL: %s\n", message); \
    } \
} while(0)

/* Color codes for output - NASA principle: Clear user feedback */
# define COLOR_RESET   "\033[0m"
# define COLOR_RED     "\033[31m"
# define COLOR_GREEN   "\033[32m"
# define COLOR_YELLOW  "\033[33m"
# define COLOR_BLUE    "\033[34m"
# define COLOR_MAGENTA "\033[35m"
# define COLOR_CYAN    "\033[36m"

/* Common test patterns - NASA principle: Predictable test data */
# define PATTERN_0x55  0x55
# define PATTERN_0xAA  0xAA
# define PATTERN_0xFF  0xFF
# define PATTERN_0x00  0x00

/* Common test sizes - NASA principle: Standard test categories */
# define SIZE_TINY     64      /* Small allocation tests */
# define SIZE_SMALL    512     /* Medium allocation tests */
# define SIZE_MEDIUM   2048    /* Large allocation tests */
# define SIZE_LARGE    8192    /* Very large allocation tests */

/* NASA principle: Maximum function complexity limits */
# define MAX_TEST_ITERATIONS    1000
# define MAX_ALLOCATION_SIZE    (1024 * 1024)  /* 1MB safety limit */
# define TEST_TIMEOUT_SECONDS   30

/* Error handling - NASA principle: Explicit error states */
typedef enum {
    TEST_SUCCESS = 0,
    TEST_FAILURE = -1,
    TEST_TIMEOUT = -2,
    TEST_CRASH = -3,
    TEST_MEMORY_ERROR = -4
} test_error_t;

/* Test configuration - NASA principle: Configurable validation */
typedef struct {
    const char *name;
    const char *description;
    int (*test_func)(void);
    int quick_test;
    int enabled;
} test_config_t;

/* Function declarations for test suites - NASA principle: Clear interfaces */
int run_malloc_tests(void);
int run_free_tests(void);
int run_realloc_tests(void);

/* Utility functions - NASA principle: Reusable components */
void test_print_header(const char *suite_name);
void test_print_summary(int passed, int failed);
int test_validate_pointer(void *ptr, size_t size);
int test_write_pattern(void *ptr, size_t size, unsigned char pattern);
int test_verify_pattern(void *ptr, size_t size, unsigned char pattern);

#endif /* TEST_COMMON_H */
