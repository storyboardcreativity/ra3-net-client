#pragma once
#include <unistd.h>
#include <sys/syscall.h>

#include "terminal_printing_system.h"
#include "printing_tools.h"

#define PROCESS_MESSAGE(level, ...)                                                                                     \
    {                                                                                                                   \
        char *buff_ptr;                                                                                                 \
        asprintf(&buff_ptr, __VA_ARGS__);                                                                               \
        tps__print_log_string(level, "[tid #%d] %s:%d\t\t|>>>| %s", syscall(SYS_gettid), __FILE__, __LINE__, buff_ptr); \
        free(buff_ptr);                                                                                                 \
    }

#define PROCESS_MESSAGE__STANDARD(...) PROCESS_MESSAGE(tps__log_string_type::standard, __VA_ARGS__)
#define PROCESS_MESSAGE__ERROR(...) PROCESS_MESSAGE(tps__log_string_type::error, __VA_ARGS__)
#define PROCESS_MESSAGE__INFO(...) PROCESS_MESSAGE(tps__log_string_type::info, __VA_ARGS__)
#define PROCESS_MESSAGE__DEBUG(...) PROCESS_MESSAGE(tps__log_string_type::debug, __VA_ARGS__)
#define PROCESS_MESSAGE__OK(...) PROCESS_MESSAGE(tps__log_string_type::ok, __VA_ARGS__)
#define PROCESS_MESSAGE__WARNING(...) PROCESS_MESSAGE(tps__log_string_type::warning, __VA_ARGS__)
#define PROCESS_MESSAGE__FATAL(...) PROCESS_MESSAGE(tps__log_string_type::fatal, __VA_ARGS__)

#define PROCESS_EXIT() exit(0);

#define PROCESS_FAILURE__WITH_MESSAGE(...)   \
    {                                        \
        PROCESS_MESSAGE__FATAL(__VA_ARGS__); \
        PROCESS_EXIT();                      \
    }

#define PROCESS_FAILURE__NOT_IMPLEMENTED_YET()                                   \
    {                                                                            \
        PROCESS_MESSAGE__FATAL("This chain is not implemented yet! Exiting..."); \
        PROCESS_EXIT();                                                          \
    }
