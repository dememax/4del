// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// open() call

#include "emulate_common.hpp"

#include <fcntl.h>

SYSTEM_CALL_OVERRIDE_BEGIN(open, const char * const pathname, int flags, ...)

    // Check if the target application is opening our emulated device
    if (pathname == EMULATED_DEVICE_ABSOLUTE_PATH) {
        std::print("[EMU] Intercepted open() for '{}', return {}\n", pathname, EMULATED_DEVICE_HANDLE);
        if (is_opened) {
            std::print("Error: '{}' is already opened\n", pathname);
            exit(2);
        }
        is_opened = true;
        return EMULATED_DEVICE_HANDLE;
    }

    std::print("[EMU] Pass through open() for '{}'\n", pathname);

    // For any other path, call the original open function
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode_t mode = va_arg(args, mode_t);
        va_end(args);
        return original_call(pathname, flags, mode);
    }

SYSTEM_CALL_OVERRIDE_END(pathname, flags)
