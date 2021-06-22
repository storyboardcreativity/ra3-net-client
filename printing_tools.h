#pragma once

#ifdef __cplusplus

#include <string>

std::string string_format(const char *fmt, ...);
std::string string_format(const char *fmt, va_list ap);
int append_format(std::string& str, const char *fmt, ...);
int append_format(std::string& str, const char *fmt, va_list ap);

#endif

int asprintf(char **strp, const char *fmt, ...);
