// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>

namespace { // Anonymous

void f_QUERYCAP(void * a)
{
    v4l2_capability & caps = *static_cast<v4l2_capability *>(a);
    const char driver[] = "Emu-v4l2";
    strncpy((char*)caps.driver, driver, sizeof caps.driver); // 16
    const char card[] = "Maxim P. Dementyev, 2025";
    strncpy((char*)caps.card, card, sizeof caps.card); // 32
    const char bus_info[] = "API Emulation using dlsym";
    strncpy((char*)caps.bus_info, bus_info, sizeof caps.bus_info); // 32
    caps.version = 1;
    caps.capabilities = V4L2_CAP_DEVICE_CAPS;
    caps.device_caps = V4L2_CAP_VIDEO_OUTPUT;
}

} // Anonymous namespace

#define CASE_REQ_ARG(name) \
    case VIDIOC_ ## name: \
        std::print("[EMU] Intercepted ioctl() for {}\n", #name); \
        { \
            va_list args; \
            va_start(args, request); \
            void* argp = va_arg(args, void*); \
            va_end(args); \
            if (!argp) { \
                std::print("[EMU] Error for intercepted ioctl() for {}: arg is null\n", #name); \
                return EINVAL; \
            } \
            f_ ## name(argp); \
        } \
        break;


SYSTEM_CALL_OVERRIDE_BEGIN(ioctl, int fd, unsigned long request, ...)

    if (EMULATED_DEVICE_HANDLE != fd) {
        std::print("[EMU] Pass through ioctl() for fd={}, request={}\n", fd, request);

        // Check the direction encoded in the request number
        if (_IOC_DIR(request) == _IOC_NONE) {
            // This ioctl takes NO third argument
            return original_call(fd, request);
        }
        // This ioctl TAKES a third argument: _IOC_READ and / or _IOC_WRITE
        va_list args;
        va_start(args, request);
        void* argp = va_arg(args, void*);
        va_end(args);
        return original_call(fd, request, argp);
    }

    switch(request) {
    CASE_REQ_ARG(QUERYCAP)
    default:
        std::print("[EMU] Unknown request {} for intercepted ioctl()\n", request);
        return EINVAL;
    }
    return 0; // success
} // ioctl() in SYSTEM_CALL_OVERRIDE_BEGIN
} // extern "C" in SYSTEM_CALL_OVERRIDE_BEGIN
