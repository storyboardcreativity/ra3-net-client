#pragma once

// OS-specific
#ifdef _MSC_VER

#include <windows.h>
#define SYS_GETTID() GetCurrentThreadId()

#elif __linux__

#include <unistd.h>
#include <sys/syscall.h>
#define SYS_GETTID() syscall(SYS_gettid)

#else

#error Unknown OS!

#endif

#include "terminal_printing_system.h"
#include "printing_tools.h"

#define PROCESS_MESSAGE(level, ...)                                                                                     \
    {                                                                                                                   \
        char *buff_ptr;                                                                                                 \
        asprintf(&buff_ptr, __VA_ARGS__);                                                                               \
        tps__print_log_string(level, "[tid #%d] %s:%d\t\t|>>>| %s", SYS_GETTID(), __FILE__, __LINE__, buff_ptr); \
        free(buff_ptr);                                                                                                 \
    }

#define PROCESS_BUFMESSAGE(level, ptr, size) tps__print_buffer(level, ptr, size)

#ifdef __cplusplus

#define PROCESS_MESSAGE__STANDARD(...) PROCESS_MESSAGE(tps__log_string_type_e::standard, __VA_ARGS__)
#define PROCESS_MESSAGE__ERROR(...) PROCESS_MESSAGE(tps__log_string_type_e::error, __VA_ARGS__)
#define PROCESS_MESSAGE__INFO(...) PROCESS_MESSAGE(tps__log_string_type_e::info, __VA_ARGS__)
#define PROCESS_MESSAGE__DEBUG(...) PROCESS_MESSAGE(tps__log_string_type_e::debug, __VA_ARGS__)
#define PROCESS_MESSAGE__OK(...) PROCESS_MESSAGE(tps__log_string_type_e::ok, __VA_ARGS__)
#define PROCESS_MESSAGE__WARNING(...) PROCESS_MESSAGE(tps__log_string_type_e::warning, __VA_ARGS__)
#define PROCESS_MESSAGE__FATAL(...) PROCESS_MESSAGE(tps__log_string_type_e::fatal, __VA_ARGS__)

#define PROCESS_BUFMESSAGE__STANDARD(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::standard, ptr, size)
#define PROCESS_BUFMESSAGE__ERROR(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::error, ptr, size)
#define PROCESS_BUFMESSAGE__INFO(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::info, ptr, size)
#define PROCESS_BUFMESSAGE__DEBUG(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::debug, ptr, size)
#define PROCESS_BUFMESSAGE__OK(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::ok, ptr, size)
#define PROCESS_BUFMESSAGE__WARNING(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::warning, ptr, size)
#define PROCESS_BUFMESSAGE__FATAL(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e::fatal, ptr, size)

#else   // For C we use special enum names (because of possible enum names collisions)

#define PROCESS_MESSAGE__STANDARD(...) PROCESS_MESSAGE(tps__log_string_type_e__standard, __VA_ARGS__)
#define PROCESS_MESSAGE__ERROR(...) PROCESS_MESSAGE(tps__log_string_type_e__error, __VA_ARGS__)
#define PROCESS_MESSAGE__INFO(...) PROCESS_MESSAGE(tps__log_string_type_e__info, __VA_ARGS__)
#define PROCESS_MESSAGE__DEBUG(...) PROCESS_MESSAGE(tps__log_string_type_e__debug, __VA_ARGS__)
#define PROCESS_MESSAGE__OK(...) PROCESS_MESSAGE(tps__log_string_type_e__ok, __VA_ARGS__)
#define PROCESS_MESSAGE__WARNING(...) PROCESS_MESSAGE(tps__log_string_type_e__warning, __VA_ARGS__)
#define PROCESS_MESSAGE__FATAL(...) PROCESS_MESSAGE(tps__log_string_type_e__fatal, __VA_ARGS__)


#define PROCESS_BUFMESSAGE__STANDARD(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__standard, ptr, size)
#define PROCESS_BUFMESSAGE__ERROR(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__error, ptr, size)
#define PROCESS_BUFMESSAGE__INFO(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__info, ptr, size)
#define PROCESS_BUFMESSAGE__DEBUG(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__debug, ptr, size)
#define PROCESS_BUFMESSAGE__OK(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__ok, ptr, size)
#define PROCESS_BUFMESSAGE__WARNING(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__warning, ptr, size)
#define PROCESS_BUFMESSAGE__FATAL(ptr, size) PROCESS_BUFMESSAGE(tps__log_string_type_e__fatal, ptr, size)

#endif

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
