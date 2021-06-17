#include <stdarg.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "printing_tools.h"

int vasprintf(char **strp, const char *fmt, va_list ap)
{
    va_list ap1;
    size_t size;
    char *buffer;

    va_copy(ap1, ap);
    size = vsnprintf(NULL, 0, fmt, ap1) + 1;
    va_end(ap1);
    buffer = (char*)calloc(1, size);

    if (!buffer)
        return -1;

    *strp = buffer;

    return vsnprintf(buffer, size, fmt, ap);
}

std::string string_format(const char *fmt, ...)
{
    char *ret;
    va_list ap;

    va_start(ap, fmt);
    vasprintf(&ret, fmt, ap);
    va_end(ap);

    std::string str(ret);
    free(ret);

    return str;
}

std::string string_format(const char *fmt, va_list ap)
{
    char *ret;
    va_list ap1;

    va_copy(ap1, ap);
    vasprintf(&ret, fmt, ap);
    va_end(ap1);

    std::string str(ret);
    free(ret);

    return str;
}

int append_format(std::string& str, const char *fmt, ...)
{
    va_list ap;
    char *ret;

    va_start(ap, fmt);
    int len = vasprintf(&ret, fmt, ap);
    va_end(ap);

    str.append(ret);
    free(ret);

    return len;
}

int append_format(std::string& str, const char *fmt, va_list ap)
{
    char *ret;
    va_list ap1;

    va_copy(ap1, ap);
    int len = vasprintf(&ret, fmt, ap);
    va_end(ap);

    str.append(ret);
    free(ret);

    return len;
}