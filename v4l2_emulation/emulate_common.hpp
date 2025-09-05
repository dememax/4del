// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// Common variables / functions

#pragma once

#include <string>

constexpr std::string EMULATED_DEVICE_ABSOLUTE_PATH("/dev/video123");
constexpr int EMULATED_DEVICE_HANDLE(987654321);
inline bool is_opened(false);

#include <dlfcn.h> // dlsym()
#include <stdlib.h> // exit()
#include <print> // print()
#include <stdarg.h> // va_list, va_start, ...

inline void * get_next_system_call_call(const std::string & n)
{
    void * const ret = dlsym(RTLD_NEXT, n.c_str());
    if (!ret) {
        const char * const err = dlerror();
        std::print("Cannot get tne next '{}()' call address: {}", n, (err ? err : "<Unknown error>"));
        exit(1);
    }
    return ret;
}
