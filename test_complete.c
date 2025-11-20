#include "include/malloc.h"
#include <unistd.h>
#include <string.h>

static void print_str(const char *str)
{
	write(1, str, strlen(str));
}

static void print_result(const char *test, int passed)
{
	print_str(test);
	print_str(": ");
	if (passed)
		print_str("✓ PASS\n");
	else
		print_str("✗ FAIL\n");
}

static int test_malloc_basic(void)
{
	void *ptr = malloc(100);
	if (!ptr)
		return 0;
	free(ptr);
	return 1;
}

static int test_malloc_tiny(void)
{
	void *ptr = malloc(64);
	if (!ptr)
		return 0;
	free(ptr);
	return 1;
}

static int test_malloc_small(void)
{
	void *ptr = malloc(512);
	if (!ptr)
		return 0;
	free(ptr);
	return 1;
}

static int test_malloc_large(void)
{
	void *ptr = malloc(2048);
	if (!ptr)
		return 0;
	free(ptr);
	return 1;
}

static int test_malloc_zero(void)
{
	void *ptr = malloc(0);
	return (ptr == NULL);
}

static int test_free_null(void)
{
	free(NULL);
	return 1;
}

static int test_multiple_allocs(void)
{
	void *ptrs[10];
	int i;

	for (i = 0; i < 10; i++) {
		ptrs[i] = malloc(100);
		if (!ptrs[i])
			return 0;
	}

	for (i = 0; i < 10; i++)
		free(ptrs[i]);

	return 1;
}

static int test_realloc_null(void)
{
	void *ptr = realloc(NULL, 100);
	if (!ptr)
		return 0;
	free(ptr);
	return 1;
}

static int test_realloc_zero(void)
{
	void *ptr = malloc(100);
	if (!ptr)
		return 0;

	void *new_ptr = realloc(ptr, 0);
	return (new_ptr == NULL);
}

static int test_realloc_grow(void)
{
	void *ptr = malloc(100);
	if (!ptr)
		return 0;

	void *new_ptr = realloc(ptr, 200);
	if (!new_ptr)
		return 0;

	free(new_ptr);
	return 1;
}

static int test_realloc_shrink(void)
{
	void *ptr = malloc(200);
	if (!ptr)
		return 0;

	void *new_ptr = realloc(ptr, 50);
	if (!new_ptr)
		return 0;

	free(new_ptr);
	return 1;
}

static int test_alignment(void)
{
	void *ptr = malloc(17);
	if (!ptr)
		return 0;

	unsigned long addr = (unsigned long)ptr;
	int aligned = (addr % 16 == 0);

	free(ptr);
	return aligned;
}

static int test_show_alloc_mem(void)
{
	void *ptr1 = malloc(64);
	void *ptr2 = malloc(512);
	void *ptr3 = malloc(2048);

	if (!ptr1 || !ptr2 || !ptr3)
		return 0;

	show_alloc_mem();

	free(ptr1);
	free(ptr2);
	free(ptr3);

	return 1;
}

static int test_malloc_cleanup(void)
{
	void *ptr1 = malloc(100);
	void *ptr2 = malloc(200);

	if (!ptr1 || !ptr2)
		return 0;

	free(ptr1);
	free(ptr2);

	int freed = malloc_cleanup();
	return (freed >= 0);
}

static int test_stress_tiny(void)
{
	void *ptrs[100];
	int i;

	for (i = 0; i < 100; i++) {
		ptrs[i] = malloc(64);
		if (!ptrs[i])
			return 0;
	}

	for (i = 0; i < 100; i++)
		free(ptrs[i]);

	return 1;
}

static int test_stress_small(void)
{
	void *ptrs[100];
	int i;

	for (i = 0; i < 100; i++) {
		ptrs[i] = malloc(512);
		if (!ptrs[i])
			return 0;
	}

	for (i = 0; i < 100; i++)
		free(ptrs[i]);

	return 1;
}

static int test_stress_large(void)
{
	void *ptrs[20];
	int i;

	for (i = 0; i < 20; i++) {
		ptrs[i] = malloc(5000);
		if (!ptrs[i])
			return 0;
	}

	for (i = 0; i < 20; i++)
		free(ptrs[i]);

	return 1;
}

static int test_fragmentation(void)
{
	void *ptrs[50];
	int i;

	for (i = 0; i < 50; i++) {
		ptrs[i] = malloc(100);
		if (!ptrs[i])
			return 0;
	}

	for (i = 0; i < 50; i += 2)
		free(ptrs[i]);

	for (i = 0; i < 25; i++) {
		ptrs[i] = malloc(100);
		if (!ptrs[i])
			return 0;
	}

	for (i = 1; i < 50; i += 2)
		free(ptrs[i]);

	for (i = 0; i < 25; i++)
		free(ptrs[i]);

	return 1;
}

static int test_double_free_protection(void)
{
	void *ptr = malloc(100);
	if (!ptr)
		return 0;

	free(ptr);
	free(ptr);

	return 1;
}

static int test_invalid_free(void)
{
	int stack_var = 42;
	free(&stack_var);
	return 1;
}

int main(void)
{
	int passed = 0;
	int total = 0;

	print_str("\n");
	print_str("=================================================\n");
	print_str("          MALLOC COMPREHENSIVE TEST SUITE        \n");
	print_str("=================================================\n");
	print_str("\n");

	print_str("Basic Functionality:\n");
	total++; if (test_malloc_basic()) passed++;
	print_result("  malloc basic", test_malloc_basic());

	total++; if (test_malloc_tiny()) passed++;
	print_result("  malloc TINY", test_malloc_tiny());

	total++; if (test_malloc_small()) passed++;
	print_result("  malloc SMALL", test_malloc_small());

	total++; if (test_malloc_large()) passed++;
	print_result("  malloc LARGE", test_malloc_large());

	total++; if (test_malloc_zero()) passed++;
	print_result("  malloc(0) returns NULL", test_malloc_zero());

	total++; if (test_free_null()) passed++;
	print_result("  free(NULL) safe", test_free_null());

	print_str("\nMultiple Allocations:\n");
	total++; if (test_multiple_allocs()) passed++;
	print_result("  10 sequential allocs/frees", test_multiple_allocs());

	print_str("\nRealloc Tests:\n");
	total++; if (test_realloc_null()) passed++;
	print_result("  realloc(NULL, size)", test_realloc_null());

	total++; if (test_realloc_zero()) passed++;
	print_result("  realloc(ptr, 0)", test_realloc_zero());

	total++; if (test_realloc_grow()) passed++;
	print_result("  realloc grow", test_realloc_grow());

	total++; if (test_realloc_shrink()) passed++;
	print_result("  realloc shrink", test_realloc_shrink());

	print_str("\nAlignment & Safety:\n");
	total++; if (test_alignment()) passed++;
	print_result("  16-byte alignment", test_alignment());

	total++; if (test_double_free_protection()) passed++;
	print_result("  double-free protection", test_double_free_protection());

	total++; if (test_invalid_free()) passed++;
	print_result("  invalid pointer free", test_invalid_free());

	print_str("\nVisualization:\n");
	total++; if (test_show_alloc_mem()) passed++;
	print_result("  show_alloc_mem()", test_show_alloc_mem());

	print_str("\nCleanup Functions:\n");
	total++; if (test_malloc_cleanup()) passed++;
	print_result("  malloc_cleanup()", test_malloc_cleanup());

	print_str("\nStress Tests:\n");
	total++; if (test_stress_tiny()) passed++;
	print_result("  100 TINY allocs", test_stress_tiny());

	total++; if (test_stress_small()) passed++;
	print_result("  100 SMALL allocs", test_stress_small());

	total++; if (test_stress_large()) passed++;
	print_result("  20 LARGE allocs", test_stress_large());

	total++; if (test_fragmentation()) passed++;
	print_result("  fragmentation handling", test_fragmentation());

	print_str("\n");
	print_str("=================================================\n");
	print_str("Results: ");

	char result[64];
	int len = 0;
	int temp_passed = passed;
	int temp_total = total;

	while (temp_passed > 0) {
		result[len++] = '0' + (temp_passed % 10);
		temp_passed /= 10;
	}
	if (len == 0) result[len++] = '0';
	while (len-- > 0) write(1, &result[len], 1);

	write(1, "/", 1);

	len = 0;
	while (temp_total > 0) {
		result[len++] = '0' + (temp_total % 10);
		temp_total /= 10;
	}
	if (len == 0) result[len++] = '0';
	while (len-- > 0) write(1, &result[len], 1);

	print_str(" tests passed\n");
	print_str("=================================================\n");
	print_str("\n");

	int leaks = check_malloc_leaks();
	if (leaks == 0) {
		print_str("✓ No memory leaks detected\n");
	} else {
		print_str("✗ Memory leaks detected\n");
	}

	print_str("\n");

	return (passed == total && leaks == 0) ? 0 : 1;
}
