#pragma once

typedef enum tps__log_string_type
{
    standard = 0,
    ok = 1,
    info = 2,
    debug = 3,
    warning = 4,
    error = 5,
    fatal = 6
} tps__log_string_type_e;

// Function is thread-safe
void tps__print_log_string(tps__log_string_type_e type, const char* fmt, ...);