#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

char *strnfcpy(char *dst, char *src, int size)
{
	char *dptr = dst, *sptr = src;
	int count = size;

	while (count--)
		if (!(*dptr++ = *sptr++)) break;

	return dst;
}

char *strnzcpy(char *dst, char *src, int size)
{
	if (size > 0) {
		*dst = 0;
		strncat(dst, src, size - 1);
	}

	return dst;
}

char *strnzcat(char *dst, char *src, int size)
{
	char *dptr = dst, *sptr = src;
	int count = size;

	if (count) {
		while (count && *dptr) {
			count--; dptr++;
		}
		if (count)
			while (--count)
				if (!(*dptr++ = *sptr++)) break;
	}
	*dptr = 0;

	return dst;
}

char *strlwr(char *s)
{
	unsigned char *ptr = (unsigned char *)s;

	while (*ptr)
	if (*ptr >= 'A' && *ptr <= 'Z')
		*ptr++ |= 0x20;
	else
		ptr++;

	return s;
}
