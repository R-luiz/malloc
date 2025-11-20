#include "../../include/malloc_internal.h"

void *ft_memcpy(void *dst, const void *src, size_t n)
{
	unsigned char *d = (unsigned char *)dst;
	const unsigned char *s = (const unsigned char *)src;
	size_t i = 0;

	while (i < n) {
		d[i] = s[i];
		i++;
	}

	return dst;
}

void *ft_memset(void *b, int c, size_t len)
{
	unsigned char *ptr = (unsigned char *)b;
	size_t i = 0;

	while (i < len) {
		ptr[i] = (unsigned char)c;
		i++;
	}

	return b;
}
