#include "../../include/malloc_internal.h"
#include <unistd.h>

static void put_char(char c)
{
	write(1, &c, 1);
}

static void put_str(const char *str)
{
	int i = 0;

	while (str[i])
		put_char(str[i++]);
}

static void put_hex(unsigned long n)
{
	char hex_digits[] = "0123456789ABCDEF";
	char buffer[16];
	int i = 0;

	if (n == 0) {
		put_char('0');
		return;
	}

	while (n > 0) {
		buffer[i++] = hex_digits[n % 16];
		n /= 16;
	}

	while (i > 0)
		put_char(buffer[--i]);
}

static void put_nbr(size_t n)
{
	char buffer[20];
	int i = 0;

	if (n == 0) {
		put_char('0');
		return;
	}

	while (n > 0) {
		buffer[i++] = '0' + (n % 10);
		n /= 10;
	}

	while (i > 0)
		put_char(buffer[--i]);
}

void print_zone_header(const char *zone_name, void *address)
{
	put_str(zone_name);
	put_str(" : 0x");
	put_hex((unsigned long)address);
	put_char('\n');
}

void print_allocation(void *start, void *end, size_t size)
{
	put_str("0x");
	put_hex((unsigned long)start);
	put_str(" - 0x");
	put_hex((unsigned long)end);
	put_str(" : ");
	put_nbr(size);
	put_str(" bytes\n");
}

void print_total(size_t total)
{
	put_str("Total : ");
	put_nbr(total);
	put_str(" bytes\n");
}
