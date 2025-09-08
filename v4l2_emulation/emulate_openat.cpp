// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// openat() call in addition to open()

#include "emulate_common.hpp"

#include <fcntl.h>

SYSTEM_CALL_OVERRIDE_BEGIN(openat, int dirfd, const char * const pathname, int flags, ...)

    // Check if the target application is opening our emulated device
    if (pathname == EMULATED_DEVICE_ABSOLUTE_PATH) {
        std::print("[EMU] Intercepted openat() for '{}', dirfd={}, redirecting to open()\n", pathname, dirfd);
        return open(pathname, flags);
    }

    std::print("[EMU] Pass through openat() for '{}', dirfd={}\n", pathname, dirfd);

    // For any other path, call the original open function
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return original_call(dirfd, pathname, flags, mode);
    }

SYSTEM_CALL_OVERRIDE_END(dirfd, pathname, flags)
