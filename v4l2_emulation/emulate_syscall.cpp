// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/syscall.h>

SYSTEM_CALL_OVERRIDE_BEGIN(syscall, long number, ...)

    // Check if the target application is opening our emulated device
    if (number == __NR_openat)
        std::print("[EMU] Intercepted syscall() for openat() call, passing through...\n");
    else
        std::print("[EMU] Pass through some syscall() with number '{}'\n", number);

    // --- IMPORTANT ---
    // For all other syscalls, we must pass them through.
    // We have to unpack all possible arguments and forward them.
    // x86-64 syscalls can have up to 6 arguments.
    va_list args;
    va_start(args, number);
    long arg1 = va_arg(args, long);
    long arg2 = va_arg(args, long);
    long arg3 = va_arg(args, long);
    long arg4 = va_arg(args, long);
    long arg5 = va_arg(args, long);
    long arg6 = va_arg(args, long);
    va_end(args);

SYSTEM_CALL_OVERRIDE_END(number, arg1, arg2, arg3, arg4, arg5, arg6)
