// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <unistd.h>

SYSTEM_CALL_OVERRIDE_BEGIN(dup, int oldfd)

    // Check if the target application is opening our emulated device
    if (oldfd == EMULATED_DEVICE_HANDLE) {
        std::print("[EMU] Intercepted dup() for {}, number_of_handles={}\n", EMULATED_DEVICE_HANDLE, number_of_handles);
        ++number_of_handles;
        return EMULATED_DEVICE_HANDLE; // success
    }

    std::print("[EMU] Pass through dup() for {}\n", oldfd);

SYSTEM_CALL_OVERRIDE_END(oldfd)
