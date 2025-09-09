// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// open() call

#include "emulate_common.hpp"

#include <stdio.h>

namespace { // Anonymous

using fopen_func_type = FILE *(*)(const char * const pathname, const char * const mode);

FILE * generic_fopen(const char * const realname, const fopen_func_type original_call,
    const char * const pathname, const char * const mode)
{
    // Check if the target application is opening our emulated device
    if (pathname == std::string("/sys/dev/char/81:123/uevent")) {
        std::print("[EMU] Intercepted {}() for '{}', mode={}, redirecting to open()\n", realname, pathname, mode);
    }

    std::print("[EMU] Pass through {}() for '{}', mode={}\n", realname, pathname, mode);

    return original_call(pathname, mode);
}

} // Anonymous namespace

#define DEFINE_FOPEN_ALTER(name) \
    SYSTEM_CALL_OVERRIDE_BEGIN_RETTYPE(FILE *, name, const char * const pathname, const char * const mode) \
        return generic_fopen(__func__, original_call, pathname, mode); \
    } \
    }

DEFINE_FOPEN_ALTER(fopen)
// DEFINE_FOPEN_ALTER(fopen64)
DEFINE_FOPEN_ALTER(__fopen64)
DEFINE_FOPEN_ALTER(__fopen)
DEFINE_FOPEN_ALTER(__fopen64_2)
DEFINE_FOPEN_ALTER(__fopen_2)
