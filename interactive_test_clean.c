#include "include/malloc.h"
#include <unistd.h>
#include <sys/time.h>

#define MAX_ALLOCS 1000

typedef struct {
	void *ptr;
	size_t size;
	int id;
	int freed;
} t_record;

static t_record g_records[MAX_ALLOCS];
static int g_count = 0;
static int g_next_id = 0;

static void put_str(const char *str)
{
	int i = 0;
	while (str[i])
		i++;
	write(1, str, i);
}

static void put_nbr(int n)
{
	char buffer[12];
	int i = 0;
	int neg = 0;

	if (n < 0) {
		neg = 1;
		n = -n;
	}

	if (n == 0) {
		write(1, "0", 1);
		return;
	}

	while (n > 0) {
		buffer[i++] = '0' + (n % 10);
		n /= 10;
	}

	if (neg)
		write(1, "-", 1);

	while (i > 0)
		write(1, &buffer[--i], 1);
}

static void put_size(size_t n)
{
	char buffer[20];
	int i = 0;

	if (n == 0) {
		write(1, "0", 1);
		return;
	}

	while (n > 0) {
		buffer[i++] = '0' + (n % 10);
		n /= 10;
	}

	while (i > 0)
		write(1, &buffer[--i], 1);
}

static void put_ptr(void *ptr)
{
	char hex_digits[] = "0123456789ABCDEF";
	char buffer[16];
	unsigned long addr = (unsigned long)ptr;
	int i = 0;

	write(1, "0x", 2);

	if (addr == 0) {
		write(1, "0", 1);
		return;
	}

	while (addr > 0) {
		buffer[i++] = hex_digits[addr % 16];
		addr /= 16;
	}

	while (i > 0)
		write(1, &buffer[--i], 1);
}

static int read_number(void)
{
	char buffer[32];
	int i = 0;
	int result = 0;
	ssize_t n;

	n = read(0, buffer, sizeof(buffer) - 1);
	if (n <= 0)
		return -1;

	while (i < n && buffer[i] >= '0' && buffer[i] <= '9') {
		result = result * 10 + (buffer[i] - '0');
		i++;
	}

	return i > 0 ? result : -1;
}

static int add_record(void *ptr, size_t size)
{
	if (g_count >= MAX_ALLOCS)
		return -1;

	g_records[g_count].ptr = ptr;
	g_records[g_count].size = size;
	g_records[g_count].id = g_next_id++;
	g_records[g_count].freed = 0;

	return g_count++;
}

static t_record *find_record(int id)
{
	int i = 0;

	while (i < g_count) {
		if (g_records[i].id == id && !g_records[i].freed)
			return &g_records[i];
		i++;
	}

	return NULL;
}

static void print_menu(void)
{
	put_str("\n");
	put_str("=============================================\n");
	put_str("        MALLOC INTERACTIVE TEST              \n");
	put_str("=============================================\n");
	put_str("\n");
	put_str("1. malloc         - Allocate memory\n");
	put_str("2. free (id)      - Free allocation\n");
	put_str("3. free all       - Free all\n");
	put_str("4. realloc (id)   - Reallocate\n");
	put_str("5. list           - List allocations\n");
	put_str("6. show_alloc_mem - Show zones\n");
	put_str("7. stats          - Statistics\n");
	put_str("8. leaks          - Check leaks\n");
	put_str("9. cleanup        - Run cleanup\n");
	put_str("0. exit           - Exit\n");
	put_str("\n");
	put_str("Active: ");
	put_nbr(g_count);
	put_str("/");
	put_nbr(MAX_ALLOCS);
	put_str("\n");
	put_str("=============================================\n");
	put_str("Command: ");
}

static void cmd_malloc(void)
{
	int size;
	int count;
	int i = 0;
	int success = 0;

	put_str("\n>>> MALLOC <<<\n");
	put_str("Size (bytes): ");
	size = read_number();
	if (size <= 0) {
		put_str("Invalid size\n");
		return;
	}

	put_str("Count (1-100): ");
	count = read_number();
	if (count <= 0 || count > 100) {
		put_str("Invalid count\n");
		return;
	}

	while (i < count) {
		void *ptr = malloc((size_t)size);
		if (ptr) {
			if (add_record(ptr, (size_t)size) >= 0)
				success++;
			else {
				free(ptr);
				break;
			}
		}
		i++;
	}

	put_str("Allocated: ");
	put_nbr(success);
	put_str("/");
	put_nbr(count);
	put_str("\n");
}

static void cmd_free_id(void)
{
	int id;
	t_record *rec;

	put_str("\n>>> FREE BY ID <<<\n");
	put_str("Allocation ID: ");
	id = read_number();

	rec = find_record(id);
	if (!rec) {
		put_str("ID not found\n");
		return;
	}

	put_str("Freeing ID ");
	put_nbr(id);
	put_str(" (");
	put_size(rec->size);
	put_str(" bytes)\n");

	free(rec->ptr);
	rec->freed = 1;
	put_str("Freed\n");
}

static void cmd_free_all(void)
{
	int i = 0;
	int count = 0;

	put_str("\n>>> FREE ALL <<<\n");

	while (i < g_count) {
		if (!g_records[i].freed) {
			free(g_records[i].ptr);
			g_records[i].freed = 1;
			count++;
		}
		i++;
	}

	put_str("Freed ");
	put_nbr(count);
	put_str(" allocations\n");
}

static void cmd_realloc(void)
{
	int id;
	int new_size;
	t_record *rec;
	void *new_ptr;

	put_str("\n>>> REALLOC <<<\n");
	put_str("Allocation ID: ");
	id = read_number();

	rec = find_record(id);
	if (!rec) {
		put_str("ID not found\n");
		return;
	}

	put_str("New size (bytes): ");
	new_size = read_number();
	if (new_size < 0) {
		put_str("Invalid size\n");
		return;
	}

	put_str("Reallocating ID ");
	put_nbr(id);
	put_str(" from ");
	put_size(rec->size);
	put_str(" to ");
	put_nbr(new_size);
	put_str(" bytes\n");

	new_ptr = realloc(rec->ptr, (size_t)new_size);
	if (new_ptr) {
		rec->ptr = new_ptr;
		rec->size = (size_t)new_size;
		put_str("Success\n");
	} else {
		put_str("Failed\n");
	}
}

static void cmd_list(void)
{
	int i = 0;
	int active = 0;
	const char *type;

	put_str("\n>>> ALLOCATIONS <<<\n");
	put_str("ID     Address           Size       Type\n");
	put_str("-------------------------------------------\n");

	while (i < g_count) {
		if (!g_records[i].freed) {
			type = "LARGE";
			if (g_records[i].size <= 128)
				type = "TINY";
			else if (g_records[i].size <= 1024)
				type = "SMALL";

			put_nbr(g_records[i].id);
			put_str("      ");
			put_ptr(g_records[i].ptr);
			put_str("  ");
			put_size(g_records[i].size);
			put_str("      ");
			put_str(type);
			put_str("\n");
			active++;
		}
		i++;
	}

	put_str("-------------------------------------------\n");
	put_str("Active: ");
	put_nbr(active);
	put_str("\n");
}

static void cmd_show_mem(void)
{
	put_str("\n>>> SHOW_ALLOC_MEM <<<\n");
	show_alloc_mem();
}

static void cmd_stats(void)
{
	t_malloc_stats stats;

	put_str("\n>>> STATISTICS <<<\n");

	if (get_malloc_stats(&stats) == 0) {
		put_str("Bytes allocated: ");
		put_size(stats.bytes_allocated);
		put_str("\n");

		put_str("TINY allocs:     ");
		put_nbr(stats.allocs_tiny);
		put_str("\n");

		put_str("SMALL allocs:    ");
		put_nbr(stats.allocs_small);
		put_str("\n");

		put_str("LARGE allocs:    ");
		put_nbr(stats.allocs_large);
		put_str("\n");
	} else {
		put_str("Failed to get stats\n");
	}
}

static void cmd_leaks(void)
{
	int leaks;

	put_str("\n>>> LEAK CHECK <<<\n");

	leaks = check_malloc_leaks();

	if (leaks > 0) {
		put_str("WARNING: ");
		put_nbr(leaks);
		put_str(" leaks detected\n");
	} else {
		put_str("No leaks\n");
	}
}

static void cmd_cleanup(void)
{
	int freed;

	put_str("\n>>> CLEANUP <<<\n");

	freed = malloc_cleanup();

	put_str("Freed ");
	put_nbr(freed);
	put_str(" empty zones\n");
}

int main(void)
{
	int cmd = -1;

	put_str("\nInteractive test started\n");
	put_str("Max allocations: ");
	put_nbr(MAX_ALLOCS);
	put_str("\n");

	while (cmd != 0) {
		print_menu();

		cmd = read_number();
		if (cmd < 0)
			continue;

		if (cmd == 1)
			cmd_malloc();
		else if (cmd == 2)
			cmd_free_id();
		else if (cmd == 3)
			cmd_free_all();
		else if (cmd == 4)
			cmd_realloc();
		else if (cmd == 5)
			cmd_list();
		else if (cmd == 6)
			cmd_show_mem();
		else if (cmd == 7)
			cmd_stats();
		else if (cmd == 8)
			cmd_leaks();
		else if (cmd == 9)
			cmd_cleanup();
		else if (cmd == 0)
			put_str("\n>>> EXIT <<<\n");
		else
			put_str("Invalid command\n");
	}

	put_str("\nFinal check:\n");
	cmd_stats();
	cmd_leaks();

	put_str("\nCleaning up...\n");
	cmd_free_all();

	put_str("\nSession ended\n\n");

	return 0;
}
