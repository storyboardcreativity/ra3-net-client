#pragma once

typedef enum tps__log_string_type
{
#ifdef __cplusplus
    standard = 0,
    ok = 1,
    info = 2,
    debug = 3,
    warning = 4,
    error = 5,
    fatal = 6
#else
    tps__log_string_type_e__standard = 0,
    tps__log_string_type_e__ok = 1,
    tps__log_string_type_e__info = 2,
    tps__log_string_type_e__debug = 3,
    tps__log_string_type_e__warning = 4,
    tps__log_string_type_e__error = 5,
    tps__log_string_type_e__fatal = 6
#endif
    
} tps__log_string_type_e;

#ifdef __cplusplus
extern "C"
{
#endif

// Function is thread-safe
void tps__print_log_string(tps__log_string_type_e type, const char* fmt, ...);

// Function is thread-safe
void tps__print_buffer(tps__log_string_type_e type, void* ptr, int size);

#ifdef __cplusplus
}
#endif