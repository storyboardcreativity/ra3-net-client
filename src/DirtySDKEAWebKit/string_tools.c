#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

// OS-specific
#ifdef _MSC_VER

#include "unistd_windows.h"

#elif __linux__

#include <unistd.h>

#else

#error Unknown OS!

#endif

char *strnfcpy(char *dst, const char *src, int size)
{
	char *dptr = dst, *sptr = (char*)src;
	int count = size;

	while (count--)
		if (!(*dptr++ = *sptr++)) break;

	return dst;
}

char *strnzcpy(char *dst, const char *src, int size)
{
	if (size > 0) {
		*dst = 0;
		strncat(dst, src, size - 1);
	}

	return dst;
}

char *strnzcat(char *dst, const char *src, int size)
{
	char *dptr = dst, *sptr = (char*)src;
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

char *strlwr(const char *s)
{
	unsigned char *ptr = (unsigned char *)s;

	while (*ptr)
	if (*ptr >= 'A' && *ptr <= 'Z')
		*ptr++ |= 0x20;
	else
		ptr++;

	return (char*)s;
}
