// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

namespace { // Anonymous

int f_QUERYCAP(void * a)
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
    return 0; // Success
}

int f_ENUMINPUT(void * a)
{
    v4l2_input & input = *static_cast<v4l2_input*>(a);
    std::print("[EMU] In ENUMINPUT, index={} - we don't have any input, we are output\n", input.index);
    // the end of the input list has been reached (empty):
    // 1) negative return value + 2) EINVAL in errno
    errno = EINVAL;
    return -1;
}

int f_G_EXT_CTRLS(void * a)
{
    v4l2_ext_controls & controls = *static_cast<v4l2_ext_controls*>(a);
    std::print("[EMU] In G_EXT_CTRLS, count={}\n", controls.count);
    errno = EINVAL;
    return -1;
}

int f_ENUMSTD(void * a)
{
    v4l2_standard & standard = *static_cast<v4l2_standard*>(a);
    std::print("[EMU] In ENUMSTD, index={}\n", standard.index);
    if (standard.index == 0) {
        bzero(&standard, sizeof standard);
        standard.id = V4L2_STD_UNKNOWN;
        const char std_name[] = "API Emul STD";
        strncpy((char*)standard.name, std_name, sizeof standard.name); // 24
        standard.frameperiod.numerator = 2;
        standard.frameperiod.denominator = 1;
        standard.framelines = 1024 * 2;
        return 0;
    }
    errno = EINVAL;
    return -1;
}

int f_QUERYCTRL(void * a)
{
    v4l2_queryctrl & control = *static_cast<v4l2_queryctrl*>(a);
    std::print("[EMU] In QUERYCTRL, id={}\n", control.id);
    errno = EINVAL;
    return -1;
}

int f_G_STD(void * a)
{
    v4l2_std_id & std_id = *static_cast<v4l2_std_id*>(a);
    std::print("[EMU] In G_STD\n");
    std_id = V4L2_STD_UNKNOWN;
    return 0;
}

int f_G_OUTPUT(void * a)
{
    int & index = *static_cast<int*>(a);
    std::print("[EMU] In G_OUTPUT\n");
    index = 0;
    return 0;
}

} // Anonymous namespace

#define CASE_REQ_ARG(name) \
    case VIDIOC_ ## name: \
        std::print("[EMU] Intercepted ioctl() for {} (#{})\n", #name, request); \
        { \
            va_list args; \
            va_start(args, request); \
            void* argp = va_arg(args, void*); \
            va_end(args); \
            if (!argp) { \
                std::print("[EMU] Error for intercepted ioctl() for {}: arg is null\n", #name); \
                errno = EINVAL; \
                return -1; \
            } \
            const int ret = f_ ## name(argp); \
            if (ret) \
                std::print("[EMU] {} result: {}, '{}' (#{})\n", #name, ret, strerror(errno), int(errno)); \
            else \
                std::print("[EMU] {} result: Success\n", #name); \
            return ret; \
        } \
        break;

#define CASE_REQ_ARG_STUB(name) \
    case VIDIOC_ ## name: \
        std::print("[EMU] Intercepted ioctl() for {} (#{}): Not implemented\n", #name, request); \
        errno = EINVAL; \
        return -1; \
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
    CASE_REQ_ARG(G_EXT_CTRLS)
    CASE_REQ_ARG_STUB(CREATE_BUFS)
    CASE_REQ_ARG_STUB(CROPCAP)
    CASE_REQ_ARG_STUB(DBG_G_CHIP_INFO)
    CASE_REQ_ARG_STUB(DBG_G_REGISTER)
    CASE_REQ_ARG_STUB(DBG_S_REGISTER)
    CASE_REQ_ARG_STUB(DECODER_CMD)
    CASE_REQ_ARG_STUB(TRY_DECODER_CMD)
    CASE_REQ_ARG_STUB(DQEVENT)
    CASE_REQ_ARG_STUB(DV_TIMINGS_CAP)
    // CASE_REQ_ARG_STUB(SUBDEV_DV_TIMINGS_CAP)
    CASE_REQ_ARG_STUB(ENCODER_CMD)
    CASE_REQ_ARG_STUB(TRY_ENCODER_CMD)
    CASE_REQ_ARG_STUB(ENUMAUDIO)
    CASE_REQ_ARG_STUB(ENUMAUDOUT)
    CASE_REQ_ARG_STUB(ENUM_DV_TIMINGS)
    // CASE_REQ_ARG_STUB(SUBDEV_ENUM_DV_TIMINGS)
    CASE_REQ_ARG_STUB(ENUM_FMT)
    CASE_REQ_ARG_STUB(ENUM_FRAMESIZES)
    CASE_REQ_ARG_STUB(ENUM_FRAMEINTERVALS)
    CASE_REQ_ARG_STUB(ENUM_FREQ_BANDS)
    CASE_REQ_ARG(ENUMINPUT)
    CASE_REQ_ARG_STUB(ENUMOUTPUT)
    CASE_REQ_ARG(ENUMSTD)
    // CASE_REQ_ARG_STUB(SUBDEV_ENUMSTD)
    CASE_REQ_ARG_STUB(EXPBUF)
    CASE_REQ_ARG_STUB(G_AUDIO)
    CASE_REQ_ARG_STUB(S_AUDIO)
    CASE_REQ_ARG_STUB(G_AUDOUT)
    CASE_REQ_ARG_STUB(S_AUDOUT)
    CASE_REQ_ARG_STUB(G_CROP)
    CASE_REQ_ARG_STUB(S_CROP)
    CASE_REQ_ARG_STUB(G_CTRL)
    CASE_REQ_ARG_STUB(S_CTRL)
    CASE_REQ_ARG_STUB(G_DV_TIMINGS)
    CASE_REQ_ARG_STUB(S_DV_TIMINGS)
    CASE_REQ_ARG_STUB(G_EDID)
    CASE_REQ_ARG_STUB(S_EDID)
    // CASE_REQ_ARG_STUB(SUBDEV_G_EDID)
    // CASE_REQ_ARG_STUB(SUBDEV_S_EDID)
    CASE_REQ_ARG_STUB(G_ENC_INDEX)
    CASE_REQ_ARG_STUB(S_EXT_CTRLS)
    CASE_REQ_ARG_STUB(TRY_EXT_CTRLS)
    CASE_REQ_ARG_STUB(G_FBUF)
    CASE_REQ_ARG_STUB(S_FBUF)
    CASE_REQ_ARG_STUB(G_FMT)
    CASE_REQ_ARG_STUB(S_FMT)
    CASE_REQ_ARG_STUB(TRY_FMT)
    CASE_REQ_ARG_STUB(G_FREQUENCY)
    CASE_REQ_ARG_STUB(S_FREQUENCY)
    CASE_REQ_ARG_STUB(G_INPUT)
    CASE_REQ_ARG_STUB(S_INPUT)
    CASE_REQ_ARG_STUB(G_JPEGCOMP)
    CASE_REQ_ARG_STUB(S_JPEGCOMP)
    CASE_REQ_ARG_STUB(G_MODULATOR)
    CASE_REQ_ARG_STUB(S_MODULATOR)
    CASE_REQ_ARG(G_OUTPUT)
    CASE_REQ_ARG_STUB(S_OUTPUT)
    CASE_REQ_ARG_STUB(G_PARM)
    CASE_REQ_ARG_STUB(S_PARM)
    CASE_REQ_ARG_STUB(G_PRIORITY)
    CASE_REQ_ARG_STUB(S_PRIORITY)
    CASE_REQ_ARG_STUB(G_SELECTION)
    CASE_REQ_ARG_STUB(S_SELECTION)
    CASE_REQ_ARG_STUB(G_SLICED_VBI_CAP)
    CASE_REQ_ARG(G_STD)
    CASE_REQ_ARG_STUB(S_STD)
    // CASE_REQ_ARG_STUB(SUBDEV_G_STD)
    // CASE_REQ_ARG_STUB(SUBDEV_S_STD)
    CASE_REQ_ARG_STUB(G_TUNER)
    CASE_REQ_ARG_STUB(S_TUNER)
    CASE_REQ_ARG_STUB(LOG_STATUS)
    CASE_REQ_ARG_STUB(OVERLAY)
    CASE_REQ_ARG_STUB(PREPARE_BUF)
    CASE_REQ_ARG_STUB(QBUF)
    CASE_REQ_ARG_STUB(DQBUF)
    CASE_REQ_ARG_STUB(QUERYBUF)
    CASE_REQ_ARG(QUERYCTRL)
    CASE_REQ_ARG_STUB(QUERY_EXT_CTRL)
    CASE_REQ_ARG_STUB(QUERYMENU)
    CASE_REQ_ARG_STUB(QUERY_DV_TIMINGS)
    CASE_REQ_ARG_STUB(QUERYSTD)
    // CASE_REQ_ARG_STUB(SUBDEV_QUERYSTD)
    CASE_REQ_ARG_STUB(REQBUFS)
    CASE_REQ_ARG_STUB(REMOVE_BUFS)
    CASE_REQ_ARG_STUB(S_HW_FREQ_SEEK)
    CASE_REQ_ARG_STUB(STREAMON)
    CASE_REQ_ARG_STUB(STREAMOFF)
    // CASE_REQ_ARG_STUB(SUBDEV_ENUM_FRAME_INTERVAL)
    // CASE_REQ_ARG_STUB(SUBDEV_ENUM_FRAME_SIZE)
    // CASE_REQ_ARG_STUB(SUBDEV_ENUM_MBUS_CODE)
    // CASE_REQ_ARG_STUB(SUBDEV_G_CROP)
    // CASE_REQ_ARG_STUB(SUBDEV_S_CROP)
    // CASE_REQ_ARG_STUB(SUBDEV_G_FMT)
    // CASE_REQ_ARG_STUB(SUBDEV_S_FMT)
    // CASE_REQ_ARG_STUB(SUBDEV_G_FRAME_INTERVAL)
    // CASE_REQ_ARG_STUB(SUBDEV_S_FRAME_INTERVAL)
    // CASE_REQ_ARG_STUB(SUBDEV_G_ROUTING)
    // CASE_REQ_ARG_STUB(SUBDEV_S_ROUTING)
    // CASE_REQ_ARG_STUB(SUBDEV_G_SELECTION)
    // CASE_REQ_ARG_STUB(SUBDEV_S_SELECTION)
    // CASE_REQ_ARG_STUB(SUBDEV_G_CLIENT_CAP)
    // CASE_REQ_ARG_STUB(SUBDEV_S_CLIENT_CAP)
    // CASE_REQ_ARG_STUB(SUBDEV_QUERYCAP)
    CASE_REQ_ARG_STUB(SUBSCRIBE_EVENT)
    CASE_REQ_ARG_STUB(UNSUBSCRIBE_EVENT)
    default:
        std::print("[EMU] Unknown request {} for intercepted ioctl()\n", request);
        errno = EINVAL;
        return -1;
    }
    return 0; // success
} // ioctl() in SYSTEM_CALL_OVERRIDE_BEGIN
} // extern "C" in SYSTEM_CALL_OVERRIDE_BEGIN
