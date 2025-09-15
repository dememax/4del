// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/mman.h>

SYSTEM_CALL_OVERRIDE_BEGIN_RETTYPE(void *, mmap, void * start, size_t length, int prot, int flags, int fd, off_t offset)

    std::print("[EMU] Pass through mmap() for start={}, length={}, prot={}, flags={}, fd={}, offset={}\n", start, length, prot, flags, fd, offset);

SYSTEM_CALL_OVERRIDE_END(start, length, prot, flags, fd, offset)
