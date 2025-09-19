// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// open() call

#include "emulate_common.hpp"

#include <fcntl.h>

namespace { // Anonymous

using open_func_type = int(*)(const char * const pathname, int flags, ...);

int generic_open(const char * const realname, const open_func_type original_call,
    const char * const pathname, int flags, ...)
{
    // Check if the target application is opening our emulated device
    if (pathname == EMULATED_DEVICE_ABSOLUTE_PATH) {
        std::print("[EMU] Intercepted {}() for '{}', return {}, number_of_handles={}\n", realname, pathname, EMULATED_DEVICE_HANDLE, number_of_handles);
        if (number_of_handles) {
            std::print("Error: '{}' is already opened\n", pathname);
            exit(2);
        }
        number_of_handles = 1;
        return EMULATED_DEVICE_HANDLE;
    }

    // For any other path, call the original open function
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        const int ret = original_call(pathname, flags, mode);
        std::print("[EMU] Pass through {}(O_CREAT) for '{}': {}\n", realname, pathname, ret);
        return ret;
    }

    const int ret = original_call(pathname, flags);
    std::print("[EMU] Pass through {}() for '{}': {}\n", realname, pathname, ret);
    return ret;
}

} // Anonymous namespace

#define DEFINE_OPEN_ALTER(name) \
    SYSTEM_CALL_OVERRIDE_BEGIN(name, const char * const pathname, int flags, ...) \
        if (flags & O_CREAT) { \
            va_list args; \
            va_start(args, flags); \
            mode_t mode = va_arg(args, mode_t); \
            va_end(args); \
            return generic_open(__func__, original_call, pathname, flags, mode); \
        } \
        return generic_open(__func__, original_call, pathname, flags); \
    } \
    }

DEFINE_OPEN_ALTER(open)
// DEFINE_OPEN_ALTER(open64)
DEFINE_OPEN_ALTER(__open64)
DEFINE_OPEN_ALTER(__open64_2)
DEFINE_OPEN_ALTER(__open_2)
