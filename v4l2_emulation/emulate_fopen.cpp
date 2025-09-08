// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// open() call

#include "emulate_common.hpp"

#include <stdio.h>

SYSTEM_CALL_OVERRIDE_BEGIN_RETTYPE(FILE *, fopen, const char * const pathname, const char * const mode)

    // Check if the target application is opening our emulated device
    if (pathname == std::string("/sys/dev/char/81:123/uevent")) {
        std::print("[EMU] Intercepted fopen() for '{}' with '{}' mode\n", pathname, mode);
    }

    std::print("[EMU] Pass through fopen() for '{}' with '{}' mode\n", pathname, mode);

SYSTEM_CALL_OVERRIDE_END(pathname, mode)
