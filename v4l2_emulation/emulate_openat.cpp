// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// openat() call in addition to open()

#include "emulate_common.hpp"

#include <fcntl.h>

namespace { // Anonymous

using openat_func_type = int(*)(int dirfd, const char * const pathname, int flags, ...);

int generic_open(const char * const realname, const openat_func_type original_call,
    int dirfd, const char * const pathname, int flags, ...)
{
    // Check if the target application is opening our emulated device
// Check if the target application is opening our emulated device
    if (pathname == EMULATED_DEVICE_ABSOLUTE_PATH) {
        std::print("[EMU] Intercepted {}() for '{}', dirfd={}, redirecting to open()\n", realname, pathname, dirfd);
        return open(pathname, flags);
    }

    std::print("[EMU] Pass through {}() for '{}', dirfd={}\n", realname, pathname, dirfd);

    // For any other path, call the original open function
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return original_call(dirfd, pathname, flags, mode);
    }
    return original_call(dirfd, pathname, flags);
}

} // Anonymous namespace

#define DEFINE_OPENAT_ALTER(name) \
    SYSTEM_CALL_OVERRIDE_BEGIN(name, int dirfd, const char * const pathname, int flags, ...) \
        if (flags & O_CREAT) { \
            va_list args; \
            va_start(args, flags); \
            mode_t mode = va_arg(args, mode_t); \
            va_end(args); \
            return generic_open(__func__, original_call, dirfd, pathname, flags, mode); \
        } \
        return generic_open(__func__, original_call, dirfd, pathname, flags); \
    } \
    }

DEFINE_OPENAT_ALTER(openat)
// DEFINE_OPENAT_ALTER(openat64)
DEFINE_OPENAT_ALTER(__openat64)
DEFINE_OPENAT_ALTER(__openat)
DEFINE_OPENAT_ALTER(__openat64_2)
DEFINE_OPENAT_ALTER(__openat_2)
