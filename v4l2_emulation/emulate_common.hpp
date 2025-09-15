// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// Common variables / functions

#pragma once

#include <string>
#include <linux/videodev2.h>

constexpr std::string EMULATED_DEVICE_ABSOLUTE_PATH("/dev/video123");
constexpr unsigned major_num = 81;
constexpr unsigned minor_num = 123;
constexpr int EMULATED_DEVICE_HANDLE(987654321);
constexpr unsigned EMULATED_PIXEL_FORMAT = V4L2_PIX_FMT_RGBA32;
constexpr unsigned EMULATED_WIDTH = 1024;
constexpr unsigned EMULATED_HEIGHT = 2048;
constexpr v4l2_fract EMULATED_FPS{1, 2};
inline unsigned number_of_handles = 0;
constexpr v4l2_pix_format EMULATED_PIXEL_FORMAT_STRUCT = {
    .width = EMULATED_WIDTH,
    .height = EMULATED_HEIGHT,
    .pixelformat = EMULATED_PIXEL_FORMAT,
    .field = V4L2_FIELD_NONE,
    .bytesperline = 4 * EMULATED_WIDTH,
    .sizeimage = /* bytesperline x height */ 4 * EMULATED_WIDTH * EMULATED_HEIGHT,
    .colorspace = V4L2_COLORSPACE_DEFAULT,
    .priv = 0,
    .flags = 0,
    .ycbcr_enc = 0,
    .quantization = V4L2_QUANTIZATION_DEFAULT,
    .xfer_func = V4L2_XFER_FUNC_DEFAULT,
};
constexpr unsigned EMULATED_BUFFER_MMAP_MAX = 2;
constexpr unsigned EMULATED_BUFFER_MMAP_OFFSETS[EMULATED_BUFFER_MMAP_MAX] = {2002002, 8008008};
inline unsigned buffers_used = 0;

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

// WHY A MACRO? A pragmatic exception to the "avoid macros" rule.
//
// PROBLEM:
// Intercepting various syscalls (like 'open', 'close', 'stat', etc.) requires highly
// repetitive boilerplate for each function:
//   1. A unique function pointer type (e.g., call_type).
//   2. A unique static variable to store the original function (e.g., original_call).
//   3. Initialization logic that uses the function's name as a string (e.g., "stat").
//
// SOLUTION:
// This macro uses the preprocessor's unique text-manipulation capabilities to
// generate all this boilerplate from a single line. It uses:
//   - Token-pasting (##) to create the unique type and variable names.
//   - Stringizing (#) to create the string literal for the lookup function.
//
// WHY NOT TEMPLATES OR CONSTEXPR?
// Modern C++ alternatives like templates are powerful but cannot achieve this.
// They operate on types and values, but they CANNOT generate new, unique
// *identifier names* (like 'open', 'close', 'stat', etc.) at compile time and
// corresponding string value to dlsym() call.
//
// This macro solves a pure code-generation problem, keeping the code DRY
// (Don't Repeat Yourself), consistent, and easier to maintain.
#define SYSTEM_CALL_OVERRIDE_BEGIN_RETTYPE(rettype, name, ...) \
    extern "C" { \
    rettype name(__VA_ARGS__) \
    { \
        using call_type = rettype(*)(__VA_ARGS__); \
        static const call_type original_call = (call_type)get_next_system_call_call(#name);

// the majority of system calls return int, so, let's make it by default
#define SYSTEM_CALL_OVERRIDE_BEGIN(name, ...) \
    SYSTEM_CALL_OVERRIDE_BEGIN_RETTYPE(int, name, __VA_ARGS__)

// Unfortunately, we cannot re-use arguments to SYSTEM_CALL_OVERRIDE_BEGIN(),
// they contain types of arguments; here, we need only names of arguments.
#define SYSTEM_CALL_OVERRIDE_END(...) \
        return original_call(__VA_ARGS__); \
    } \
    } // extern "C"
