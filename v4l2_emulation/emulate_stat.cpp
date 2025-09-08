// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/stat.h>

SYSTEM_CALL_OVERRIDE_BEGIN(stat, const char * pathname, struct stat * statbuf)

    // Check if the target application is opening our emulated device
    if (pathname == EMULATED_DEVICE_ABSOLUTE_PATH) {
        std::print("[EMU] Intercepted stat() for '{}'\n", pathname);
        if (!statbuf) {
            std::print("[EMU] Error the pointer to struct stat for '{}' is NULL.\n", pathname);
            return EFAULT;
        }
        statbuf->st_mode = S_IFCHR;
        return 0; // success
    }

    std::print("[EMU] Pass through open() for '{}'\n", pathname);

SYSTEM_CALL_OVERRIDE_END(pathname, statbuf)
