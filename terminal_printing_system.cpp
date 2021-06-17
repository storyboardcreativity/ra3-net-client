#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include "terminal_printing_system.h"
#include "printing_tools.h"

pthread_mutex_t g_terminal_printing_system__screen_mutex = PTHREAD_MUTEX_INITIALIZER;

void tps__print_log_string(tps__log_string_type_e type, const char* fmt, ...)
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