// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/mman.h>
#include <errno.h>
#include <cstdint>

SYSTEM_CALL_OVERRIDE_BEGIN_RETTYPE(void *, mmap, void * start, size_t length, int prot, int flags, int fd, off_t offset)

    if (EMULATED_DEVICE_HANDLE != fd) {
        std::print("[EMU] Pass through mmap() for start={}, length={}, prot={}, flags={}, fd={}, offset={}\n", start, length, prot, flags, fd, offset);
        return original_call(start, length, prot, flags, fd, offset);
    }
    std::print("[EMU] Intercepted mmap() for start={}, length={}, prot={}, flags={}, offset={}\n", start, length, prot, flags, offset);

    if (start) {
        std::print("[EMU] Wrong param to mmap(): expected start=NULL\n");
        errno = EINVAL;
        return MAP_FAILED;
    }
    if (length != EMULATED_PIXEL_FORMAT_STRUCT.sizeimage) {
        std::print("[EMU] Wrong param to mmap(): expected length={}\n", EMULATED_PIXEL_FORMAT_STRUCT.sizeimage);
        errno = EINVAL;
        return MAP_FAILED;
    }
    if (prot != (PROT_READ | PROT_WRITE)) {
        std::print("[EMU] Wrong param to mmap(): expected prot=PROT_READ|PROT_WRITE ({}|{})\n", int(PROT_READ), int(PROT_WRITE));
        errno = EINVAL;
        return MAP_FAILED;
    }
    if (flags != MAP_SHARED) {
        std::print("[EMU] Wrong param to mmap(): expected flags=MAP_SHARED ({})\n", int(MAP_SHARED));
        errno = EINVAL;
        return MAP_FAILED;
    }

    for (unsigned i=0; i<EMULATED_BUFFER_MMAP_MAX; ++i)
    {
        if (EMULATED_BUFFER_MMAP_OFFSETS[i] == offset) {
            auto & vec(EMULATED_BUFFER_MMAP_BUFFERS[i]);
            vec.resize(EMULATED_PIXEL_FORMAT_STRUCT.sizeimage);
            void * const ret(vec.data());
            std::print("[EMU] Found buffer #{}: {}\n", i, ret);
            return ret;
        }
    }

    std::print("[EMU] Wrong param to mmap(): unexpected offset\n");
    errno = EINVAL;
    return MAP_FAILED;
} // ioctl() in SYSTEM_CALL_OVERRIDE_BEGIN
} // extern "C" in SYSTEM_CALL_OVERRIDE_BEGIN
