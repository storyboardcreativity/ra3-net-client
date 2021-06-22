#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "terminal_printing_system.h"
#include "printing_tools.h"

pthread_mutex_t g_terminal_printing_system__screen_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

std::vector<std::string> convert_buffer_to_printable_strings(const void* ptr, int size)
{
    const int symbols_cnt = 16;

    std::vector<std::string> strings;
    strings.reserve(size);

    std::stringstream stream;
    
	auto data = (uint8_t*)ptr;
	for (int i = 0; i < size; i += symbols_cnt)
	{
		for (int p = 0; p < symbols_cnt; ++p)
		{
			if (i + p < size)
				stream << std::hex << std::setw(2) << std::setfill('0') << (int)data[i + p] << " ";
			else
				stream << ".. ";
		}

		stream << "| ";

		for (int p = 0; p < symbols_cnt; ++p)
		{
			if (i + p < size)
				stream << (char)((isgraph(data[i + p])) ? data[i + p] : '.');
			else
				stream << " ";
		}

		stream << std::setw(0) << std::dec;

        strings.push_back(stream.str());
        stream.str(std::string());
	}

    return strings;
}

extern "C" void tps__print_log_string(tps__log_string_type_e type, const char* fmt, ...)
{
    pthread_mutex_lock(&g_terminal_printing_system__screen_mutex);

    switch(type)
    {
        case tps__log_string_type_e::fatal:
            // Set background color to RED
            printf("\033[41m");

            // Set text color to YELLOW
            printf("\033[33m");
            break;
            
        case tps__log_string_type_e::error:
            // Set text color to RED
            printf("\033[0;31m");
            break;

        case tps__log_string_type_e::debug:
            // Set text color to MAGENTA
            printf("\033[0;35m");
            break;

        case tps__log_string_type_e::info:
            // Set text color to CYAN
            printf("\033[0;36m");
            break;

        case tps__log_string_type_e::warning:
            // Set text color to YELLOW
            printf("\033[0;33m");
            break;
        
        case tps__log_string_type_e::ok:
            // Set text color to GREEN
            printf("\033[0;32m");
            break;

        case tps__log_string_type_e::standard:
        default:
            printf("\033[0m");
    }

    // Print our string
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    // Reset text to default color
    printf("\033[0m");

    // Add new line
    printf("\n");

    pthread_mutex_unlock(&g_terminal_printing_system__screen_mutex);
}

extern "C" void tps__print_buffer(tps__log_string_type_e type, const void* ptr, int size)
{
    if (size < 0)
        return;
        
    pthread_mutex_lock(&g_terminal_printing_system__screen_mutex);

    auto strings = convert_buffer_to_printable_strings(ptr, size);
    for (int i = 0; i < strings.size(); ++i)
    {
        tps__print_log_string(type, "%s", strings[i].c_str());
    }

    pthread_mutex_unlock(&g_terminal_printing_system__screen_mutex);
}