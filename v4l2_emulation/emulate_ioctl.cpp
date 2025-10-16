// Maxim P. DEMENTYEV, 2025
// Video for Linux ver 2 emulation
// stat() system call for a v4l2 device

#include "emulate_common.hpp"

#include <sys/ioctl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <format>

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
    caps.device_caps = V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_STREAMING;
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
        standard.frameperiod = EMULATED_FPS;
        standard.framelines = EMULATED_HEIGHT;
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

template<typename T>
std::string v4l2_buf_type_to_str(const T t)
{
#define CASE_BUF_TYPE(n) case V4L2_BUF_TYPE_ ## n: return #n;
    switch(static_cast<v4l2_buf_type>(t)) {
    CASE_BUF_TYPE(VIDEO_CAPTURE)
    CASE_BUF_TYPE(VIDEO_OUTPUT)
    CASE_BUF_TYPE(VIDEO_OVERLAY)
    CASE_BUF_TYPE(VBI_CAPTURE)
    CASE_BUF_TYPE(VBI_OUTPUT)
    CASE_BUF_TYPE(SLICED_VBI_CAPTURE)
    CASE_BUF_TYPE(SLICED_VBI_OUTPUT)
    CASE_BUF_TYPE(VIDEO_OUTPUT_OVERLAY)
    CASE_BUF_TYPE(VIDEO_CAPTURE_MPLANE)
    CASE_BUF_TYPE(VIDEO_OUTPUT_MPLANE)
    CASE_BUF_TYPE(SDR_CAPTURE)
    CASE_BUF_TYPE(SDR_OUTPUT)
    CASE_BUF_TYPE(META_CAPTURE)
    CASE_BUF_TYPE(META_OUTPUT)
    CASE_BUF_TYPE(PRIVATE)
    default:
        return std::format("Unknown #{}", int(t));
    }
#undef CASE_BUF_TYPE
}

std::string v4l2_format_to_string(unsigned format) {
    char fourcc[5] = {
        static_cast<char>(format & 0xFF),
        static_cast<char>((format >> 8) & 0xFF),
        static_cast<char>((format >> 16) & 0xFF),
        static_cast<char>((format >> 24) & 0xFF),
        '\0'
    };
    return std::string(fourcc);
}

int f_ENUM_FMT(void * a)
{
    v4l2_fmtdesc & format = *static_cast<v4l2_fmtdesc*>(a);
    std::print("[EMU] In ENUM_FMT: index={}, type={}, mbus_code={}\n", format.index, v4l2_buf_type_to_str(format.type), format.mbus_code);
    // we expose only one output format for this output
    if (format.index != 0 or format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT or format.mbus_code != 0) {
        errno = EINVAL;
        return -1;
    }
    format.flags = 0;
    const char fmt_desc[] = "32-bit RGBA 8-8-8-8";
    strncpy((char*)format.description, fmt_desc, sizeof format.description); // 32
    format.pixelformat = EMULATED_PIXEL_FORMAT;
    return 0;
}

int f_CROPCAP(void * a)
{
    v4l2_cropcap & cropcap = *static_cast<v4l2_cropcap*>(a);
    std::print("[EMU] In CROPCAP: type={}\n", v4l2_buf_type_to_str(cropcap.type));
    // we expose only one output format for this output
    if (cropcap.type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
        errno = EINVAL;
        return -1;
    }
    constexpr v4l2_rect rect{0, 0, EMULATED_WIDTH, EMULATED_HEIGHT};
    cropcap.bounds = rect;
    cropcap.defrect = rect;
    constexpr v4l2_fract aspect{1, 1};
    cropcap.pixelaspect = aspect;
    return 0;
}

int f_ENUM_FRAMESIZES(void * a)
{
    v4l2_frmsizeenum & frame_size = *static_cast<v4l2_frmsizeenum*>(a);
    std::print("[EMU] In ENUM_FRAMESIZES: index={}, pixel_format={}\n",
        frame_size.index, v4l2_format_to_string(frame_size.pixel_format));
    // we expose only one Discrete output format
    if (frame_size.index != 0 or frame_size.pixel_format != EMULATED_PIXEL_FORMAT) {
        errno = EINVAL;
        return -1;
    }
    frame_size.type = V4L2_FRMSIZE_TYPE_DISCRETE;
    constexpr v4l2_frmsize_discrete sz{EMULATED_WIDTH, EMULATED_HEIGHT};
    frame_size.discrete = sz;
    return 0;
}

int f_G_FMT(void * a)
{
    v4l2_format & format = *static_cast<v4l2_format*>(a);
    std::print("[EMU] In G_FMT: type={}\n", v4l2_buf_type_to_str(format.type));
    // we expose only one output format for this output
    if (format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
        errno = EINVAL;
        return -1;
    }
    format.fmt.pix = EMULATED_PIXEL_FORMAT_STRUCT;
    return 0;
}

int generic_change_format(bool is_try, void * a)
{
    const char * const n = is_try ? "TRY" : "S";
    // cannot be const, we must adjust value of bytesperline and sizeimage
    v4l2_format & format = *static_cast<v4l2_format*>(a);
    std::print("[EMU] In {}_FMT: type={}\n", n, v4l2_buf_type_to_str(format.type));
    // we expose only one output format for this output
    if (format.type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
        errno = EINVAL;
        return -1;
    }
    v4l2_pix_format & pix_fmt(format.fmt.pix);
#define CHECK_PIXEL_FORMAT_STRUCT_MEMBER(f) \
    if (pix_fmt.f != EMULATED_PIXEL_FORMAT_STRUCT.f) { \
        std::print("[EMU] Check failed for {}_FMT, {}: expected {}, got {}\n", \
            n, #f, EMULATED_PIXEL_FORMAT_STRUCT.f, pix_fmt.f); \
        errno = EINVAL; \
        return -1; \
    }
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(width);
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(height);
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(pixelformat);
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(field);
    pix_fmt = EMULATED_PIXEL_FORMAT_STRUCT;
#undef CHECK_PIXEL_FORMAT_STRUCT_MEMBER
    return 0;
}

int f_TRY_FMT(void * a)
{
    return generic_change_format(true, a);
}

int f_S_FMT(void * a)
{
    return generic_change_format(false, a);
}

int f_ENUM_FRAMEINTERVALS(void * a)
{
    v4l2_frmivalenum & interval = *static_cast<v4l2_frmivalenum*>(a);
    std::print("[EMU] In ENUM_FRAMEINTERVALS: index={}, pixel_format={}, width={}, height={}\n",
        interval.index, v4l2_format_to_string(interval.pixel_format), interval.width, interval.height);
    // we expose only one output format for this output
    if (interval.index != 0 or interval.pixel_format != EMULATED_PIXEL_FORMAT or
            interval.width != EMULATED_WIDTH or interval.height != EMULATED_HEIGHT) {
        errno = EINVAL;
        return -1;
    }
    interval.type = V4L2_FRMIVAL_TYPE_DISCRETE;
    interval.discrete = EMULATED_FPS;
    return 0;
}

constexpr const char * v4l2_control_id_to_c_str(int id)
{
#define CASE_CONTROL_ID(n) case V4L2_CID_ ## n: return #n;
    switch(id) {
    CASE_CONTROL_ID(USER_CLASS)
    CASE_CONTROL_ID(BRIGHTNESS) // V4L2_CID_USER_BASE, V4L2_CID_BASE
    CASE_CONTROL_ID(CONTRAST)
    CASE_CONTROL_ID(SATURATION)
    CASE_CONTROL_ID(HUE)
    CASE_CONTROL_ID(AUDIO_VOLUME)
    CASE_CONTROL_ID(AUDIO_BALANCE)
    CASE_CONTROL_ID(AUDIO_BASS)
    CASE_CONTROL_ID(AUDIO_TREBLE)
    CASE_CONTROL_ID(AUDIO_MUTE)
    CASE_CONTROL_ID(AUDIO_LOUDNESS)
    CASE_CONTROL_ID(BLACK_LEVEL)
    CASE_CONTROL_ID(AUTO_WHITE_BALANCE)
    CASE_CONTROL_ID(DO_WHITE_BALANCE)
    CASE_CONTROL_ID(RED_BALANCE)
    CASE_CONTROL_ID(BLUE_BALANCE)
    CASE_CONTROL_ID(GAMMA) // V4L2_CID_WHITENESS
    CASE_CONTROL_ID(EXPOSURE)
    CASE_CONTROL_ID(AUTOGAIN)
    CASE_CONTROL_ID(GAIN)
    CASE_CONTROL_ID(HFLIP)
    CASE_CONTROL_ID(VFLIP)
    CASE_CONTROL_ID(POWER_LINE_FREQUENCY)
    CASE_CONTROL_ID(HUE_AUTO)
    CASE_CONTROL_ID(WHITE_BALANCE_TEMPERATURE)
    CASE_CONTROL_ID(SHARPNESS)
    CASE_CONTROL_ID(BACKLIGHT_COMPENSATION)
    CASE_CONTROL_ID(CHROMA_AGC)
    CASE_CONTROL_ID(COLOR_KILLER)
    CASE_CONTROL_ID(COLORFX)
    CASE_CONTROL_ID(AUTOBRIGHTNESS)
    CASE_CONTROL_ID(BAND_STOP_FILTER)
    CASE_CONTROL_ID(ROTATE)
    CASE_CONTROL_ID(BG_COLOR)
    CASE_CONTROL_ID(CHROMA_GAIN)
    CASE_CONTROL_ID(ILLUMINATORS_1)
    CASE_CONTROL_ID(ILLUMINATORS_2)
    CASE_CONTROL_ID(MIN_BUFFERS_FOR_CAPTURE)
    CASE_CONTROL_ID(MIN_BUFFERS_FOR_OUTPUT)
    CASE_CONTROL_ID(ALPHA_COMPONENT)
    CASE_CONTROL_ID(COLORFX_CBCR)
    CASE_CONTROL_ID(COLORFX_RGB)
    CASE_CONTROL_ID(LASTP1)
    CASE_CONTROL_ID(USER_MEYE_BASE)
    CASE_CONTROL_ID(USER_BTTV_BASE)
    CASE_CONTROL_ID(USER_S2255_BASE)
    CASE_CONTROL_ID(USER_SI476X_BASE)
    CASE_CONTROL_ID(USER_TI_VPE_BASE)
    CASE_CONTROL_ID(USER_SAA7134_BASE)
    CASE_CONTROL_ID(USER_ADV7180_BASE)
    CASE_CONTROL_ID(USER_TC358743_BASE)
    CASE_CONTROL_ID(USER_MAX217X_BASE)
    CASE_CONTROL_ID(USER_IMX_BASE)
    CASE_CONTROL_ID(USER_ATMEL_ISC_BASE)
    CASE_CONTROL_ID(USER_CODA_BASE)
    CASE_CONTROL_ID(USER_CCS_BASE)
    CASE_CONTROL_ID(USER_ALLEGRO_BASE)
    CASE_CONTROL_ID(USER_ISL7998X_BASE)
    CASE_CONTROL_ID(USER_DW100_BASE)
    CASE_CONTROL_ID(USER_ASPEED_BASE)
    CASE_CONTROL_ID(USER_NPCM_BASE)
    CASE_CONTROL_ID(USER_THP7312_BASE)
    CASE_CONTROL_ID(CODEC_CLASS)
    CASE_CONTROL_ID(MPEG_STREAM_TYPE) // V4L2_CID_CODEC_BASE
    CASE_CONTROL_ID(MPEG_STREAM_PID_PMT)
    CASE_CONTROL_ID(MPEG_STREAM_PID_AUDIO)
    CASE_CONTROL_ID(MPEG_STREAM_PID_VIDEO)
    CASE_CONTROL_ID(MPEG_STREAM_PID_PCR)
    CASE_CONTROL_ID(MPEG_STREAM_PES_ID_AUDIO)
    CASE_CONTROL_ID(MPEG_STREAM_PES_ID_VIDEO)
    CASE_CONTROL_ID(MPEG_STREAM_VBI_FMT)
    CASE_CONTROL_ID(MPEG_AUDIO_SAMPLING_FREQ)
    CASE_CONTROL_ID(MPEG_AUDIO_ENCODING)
    CASE_CONTROL_ID(MPEG_AUDIO_L1_BITRATE)
    CASE_CONTROL_ID(MPEG_AUDIO_L2_BITRATE)
    CASE_CONTROL_ID(MPEG_AUDIO_L3_BITRATE)
    CASE_CONTROL_ID(MPEG_AUDIO_MODE)
    CASE_CONTROL_ID(MPEG_AUDIO_MODE_EXTENSION)
    CASE_CONTROL_ID(MPEG_AUDIO_EMPHASIS)
    CASE_CONTROL_ID(MPEG_AUDIO_CRC)
    CASE_CONTROL_ID(MPEG_AUDIO_MUTE)
    CASE_CONTROL_ID(MPEG_AUDIO_AAC_BITRATE)
    CASE_CONTROL_ID(MPEG_AUDIO_AC3_BITRATE)
    CASE_CONTROL_ID(MPEG_AUDIO_DEC_PLAYBACK)
    CASE_CONTROL_ID(MPEG_AUDIO_DEC_MULTILINGUAL_PLAYBACK)
    CASE_CONTROL_ID(MPEG_VIDEO_ENCODING)
    CASE_CONTROL_ID(MPEG_VIDEO_ASPECT)
    CASE_CONTROL_ID(MPEG_VIDEO_B_FRAMES)
    CASE_CONTROL_ID(MPEG_VIDEO_GOP_SIZE)
    CASE_CONTROL_ID(MPEG_VIDEO_GOP_CLOSURE)
    CASE_CONTROL_ID(MPEG_VIDEO_PULLDOWN)
    CASE_CONTROL_ID(MPEG_VIDEO_BITRATE_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_BITRATE)
    CASE_CONTROL_ID(MPEG_VIDEO_BITRATE_PEAK)
    CASE_CONTROL_ID(MPEG_VIDEO_TEMPORAL_DECIMATION)
    CASE_CONTROL_ID(MPEG_VIDEO_MUTE)
    CASE_CONTROL_ID(MPEG_VIDEO_MUTE_YUV)
    CASE_CONTROL_ID(MPEG_VIDEO_DECODER_SLICE_INTERFACE)
    CASE_CONTROL_ID(MPEG_VIDEO_DECODER_MPEG4_DEBLOCK_FILTER)
    CASE_CONTROL_ID(MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB)
    CASE_CONTROL_ID(MPEG_VIDEO_FRAME_RC_ENABLE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEADER_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_MAX_REF_PIC)
    CASE_CONTROL_ID(MPEG_VIDEO_MB_RC_ENABLE)
    CASE_CONTROL_ID(MPEG_VIDEO_MULTI_SLICE_MAX_BYTES)
    CASE_CONTROL_ID(MPEG_VIDEO_MULTI_SLICE_MAX_MB)
    CASE_CONTROL_ID(MPEG_VIDEO_MULTI_SLICE_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_VBV_SIZE)
    CASE_CONTROL_ID(MPEG_VIDEO_DEC_PTS)
    CASE_CONTROL_ID(MPEG_VIDEO_DEC_FRAME)
    CASE_CONTROL_ID(MPEG_VIDEO_VBV_DELAY)
    CASE_CONTROL_ID(MPEG_VIDEO_REPEAT_SEQ_HEADER)
    CASE_CONTROL_ID(MPEG_VIDEO_MV_H_SEARCH_RANGE)
    CASE_CONTROL_ID(MPEG_VIDEO_MV_V_SEARCH_RANGE)
    CASE_CONTROL_ID(MPEG_VIDEO_FORCE_KEY_FRAME)
    CASE_CONTROL_ID(MPEG_VIDEO_BASELAYER_PRIORITY_ID)
    CASE_CONTROL_ID(MPEG_VIDEO_AU_DELIMITER)
    CASE_CONTROL_ID(MPEG_VIDEO_LTR_COUNT)
    CASE_CONTROL_ID(MPEG_VIDEO_FRAME_LTR_INDEX)
    CASE_CONTROL_ID(MPEG_VIDEO_USE_LTR_FRAMES)
    CASE_CONTROL_ID(MPEG_VIDEO_DEC_CONCEAL_COLOR)
    CASE_CONTROL_ID(MPEG_VIDEO_INTRA_REFRESH_PERIOD)
    CASE_CONTROL_ID(MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG2_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG2_PROFILE)
    CASE_CONTROL_ID(FWHT_I_FRAME_QP)
    CASE_CONTROL_ID(FWHT_P_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H263_I_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H263_P_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H263_B_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H263_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H263_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_I_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_P_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_B_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_8X8_TRANSFORM)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_CPB_SIZE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_ENTROPY_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_I_PERIOD)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_LOOP_FILTER_ALPHA)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_LOOP_FILTER_BETA)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_LOOP_FILTER_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_PROFILE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_VUI_SAR_ENABLE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_VUI_SAR_IDC)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_SEI_FRAME_PACKING)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_SEI_FP_CURRENT_FRAME_0)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_SEI_FP_ARRANGEMENT_TYPE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_FMO)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_FMO_MAP_TYPE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_FMO_SLICE_GROUP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_FMO_CHANGE_DIRECTION)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_FMO_CHANGE_RATE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_FMO_RUN_LENGTH)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_ASO)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_ASO_SLICE_ORDER)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIERARCHICAL_CODING)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_I_FRAME_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_I_FRAME_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_P_FRAME_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_P_FRAME_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_B_FRAME_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_B_FRAME_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L0_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L1_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L2_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L3_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L4_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L5_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_H264_HIER_CODING_L6_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_I_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_P_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_B_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_PROFILE)
    CASE_CONTROL_ID(MPEG_VIDEO_MPEG4_QPEL)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_NUM_PARTITIONS)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_IMD_DISABLE_4X4)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_NUM_REF_FRAMES)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_FILTER_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_FILTER_SHARPNESS)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_GOLDEN_FRAME_SEL)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_I_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_VPX_P_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_VP8_PROFILE)
    CASE_CONTROL_ID(MPEG_VIDEO_VP9_PROFILE)
    CASE_CONTROL_ID(MPEG_VIDEO_VP9_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_I_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_P_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_B_FRAME_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_TYPE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_LAYER)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L0_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L1_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L2_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L3_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L4_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L5_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L6_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_PROFILE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_FRAME_RATE_RESOLUTION)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_TIER)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_MAX_PARTITION_DEPTH)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_LOOP_FILTER_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_REFRESH_TYPE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_REFRESH_PERIOD)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_LOSSLESS_CU)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_CONST_INTRA_PRED)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_WAVEFRONT)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_GENERAL_PB)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_TEMPORAL_ID)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_STRONG_SMOOTHING)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_MAX_NUM_MERGE_MV_MINUS1)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_INTRA_PU_SPLIT)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_TMV_PREDICTION)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_WITHOUT_STARTCODE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L0_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L1_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L2_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L3_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L4_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L5_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_HIER_CODING_L6_BR)
    CASE_CONTROL_ID(MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES)
    CASE_CONTROL_ID(MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR)
    CASE_CONTROL_ID(MPEG_VIDEO_CONSTANT_QUALITY)
    CASE_CONTROL_ID(MPEG_VIDEO_FRAME_SKIP_MODE)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_I_FRAME_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_I_FRAME_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_P_FRAME_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_P_FRAME_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_B_FRAME_MIN_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_HEVC_B_FRAME_MAX_QP)
    CASE_CONTROL_ID(MPEG_VIDEO_DEC_DISPLAY_DELAY)
    CASE_CONTROL_ID(MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE)
    CASE_CONTROL_ID(MPEG_VIDEO_AV1_PROFILE)
    CASE_CONTROL_ID(MPEG_VIDEO_AV1_LEVEL)
    CASE_CONTROL_ID(MPEG_VIDEO_AVERAGE_QP)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_SPATIAL_FILTER_MODE) // V4L2_CID_CODEC_CX2341X_BASE
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_SPATIAL_FILTER)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_CHROMA_SPATIAL_FILTER_TYPE)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_TEMPORAL_FILTER_MODE)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_TEMPORAL_FILTER)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_MEDIAN_FILTER_TYPE)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_BOTTOM)
    CASE_CONTROL_ID(MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_TOP)
    CASE_CONTROL_ID(MPEG_CX2341X_STREAM_INSERT_NAV_PACKETS)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY) // V4L2_CID_CODEC_MFC51_BASE
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_FRAME_SKIP_MODE)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_PADDING)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_PADDING_YUV)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_RC_FIXED_TARGET_BIT)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_RC_REACTION_COEFF)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_ACTIVITY)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_DARK)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_SMOOTH)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_STATIC)
    CASE_CONTROL_ID(MPEG_MFC51_VIDEO_H264_NUM_REF_PIC_FOR_P)
    CASE_CONTROL_ID(CAMERA_CLASS_BASE)
    CASE_CONTROL_ID(CAMERA_CLASS)
    CASE_CONTROL_ID(EXPOSURE_AUTO)
    CASE_CONTROL_ID(EXPOSURE_ABSOLUTE)
    CASE_CONTROL_ID(EXPOSURE_AUTO_PRIORITY)
    CASE_CONTROL_ID(PAN_RELATIVE)
    CASE_CONTROL_ID(TILT_RELATIVE)
    CASE_CONTROL_ID(PAN_RESET)
    CASE_CONTROL_ID(TILT_RESET)
    CASE_CONTROL_ID(PAN_ABSOLUTE)
    CASE_CONTROL_ID(TILT_ABSOLUTE)
    CASE_CONTROL_ID(FOCUS_ABSOLUTE)
    CASE_CONTROL_ID(FOCUS_RELATIVE)
    CASE_CONTROL_ID(FOCUS_AUTO)
    CASE_CONTROL_ID(ZOOM_ABSOLUTE)
    CASE_CONTROL_ID(ZOOM_RELATIVE)
    CASE_CONTROL_ID(ZOOM_CONTINUOUS)
    CASE_CONTROL_ID(PRIVACY)
    CASE_CONTROL_ID(IRIS_ABSOLUTE)
    CASE_CONTROL_ID(IRIS_RELATIVE)
    CASE_CONTROL_ID(AUTO_EXPOSURE_BIAS)
    CASE_CONTROL_ID(AUTO_N_PRESET_WHITE_BALANCE)
    CASE_CONTROL_ID(WIDE_DYNAMIC_RANGE)
    CASE_CONTROL_ID(IMAGE_STABILIZATION)
    CASE_CONTROL_ID(ISO_SENSITIVITY)
    CASE_CONTROL_ID(ISO_SENSITIVITY_AUTO)
    CASE_CONTROL_ID(EXPOSURE_METERING)
    CASE_CONTROL_ID(SCENE_MODE)
    CASE_CONTROL_ID(3A_LOCK)
    CASE_CONTROL_ID(AUTO_FOCUS_START)
    CASE_CONTROL_ID(AUTO_FOCUS_STOP)
    CASE_CONTROL_ID(AUTO_FOCUS_STATUS)
    CASE_CONTROL_ID(AUTO_FOCUS_RANGE)
    CASE_CONTROL_ID(PAN_SPEED)
    CASE_CONTROL_ID(TILT_SPEED)
    CASE_CONTROL_ID(CAMERA_ORIENTATION)
    CASE_CONTROL_ID(CAMERA_SENSOR_ROTATION)
    CASE_CONTROL_ID(HDR_SENSOR_MODE)
    CASE_CONTROL_ID(FM_TX_CLASS_BASE)
    CASE_CONTROL_ID(FM_TX_CLASS)
    CASE_CONTROL_ID(RDS_TX_DEVIATION)
    CASE_CONTROL_ID(RDS_TX_PI)
    CASE_CONTROL_ID(RDS_TX_PTY)
    CASE_CONTROL_ID(RDS_TX_PS_NAME)
    CASE_CONTROL_ID(RDS_TX_RADIO_TEXT)
    CASE_CONTROL_ID(RDS_TX_MONO_STEREO)
    CASE_CONTROL_ID(RDS_TX_ARTIFICIAL_HEAD)
    CASE_CONTROL_ID(RDS_TX_COMPRESSED)
    CASE_CONTROL_ID(RDS_TX_DYNAMIC_PTY)
    CASE_CONTROL_ID(RDS_TX_TRAFFIC_ANNOUNCEMENT)
    CASE_CONTROL_ID(RDS_TX_TRAFFIC_PROGRAM)
    CASE_CONTROL_ID(RDS_TX_MUSIC_SPEECH)
    CASE_CONTROL_ID(RDS_TX_ALT_FREQS_ENABLE)
    CASE_CONTROL_ID(RDS_TX_ALT_FREQS)
    CASE_CONTROL_ID(AUDIO_LIMITER_ENABLED)
    CASE_CONTROL_ID(AUDIO_LIMITER_RELEASE_TIME)
    CASE_CONTROL_ID(AUDIO_LIMITER_DEVIATION)
    CASE_CONTROL_ID(AUDIO_COMPRESSION_ENABLED)
    CASE_CONTROL_ID(AUDIO_COMPRESSION_GAIN)
    CASE_CONTROL_ID(AUDIO_COMPRESSION_THRESHOLD)
    CASE_CONTROL_ID(AUDIO_COMPRESSION_ATTACK_TIME)
    CASE_CONTROL_ID(AUDIO_COMPRESSION_RELEASE_TIME)
    CASE_CONTROL_ID(PILOT_TONE_ENABLED)
    CASE_CONTROL_ID(PILOT_TONE_DEVIATION)
    CASE_CONTROL_ID(PILOT_TONE_FREQUENCY)
    CASE_CONTROL_ID(TUNE_PREEMPHASIS)
    CASE_CONTROL_ID(TUNE_POWER_LEVEL)
    CASE_CONTROL_ID(TUNE_ANTENNA_CAPACITOR)
    CASE_CONTROL_ID(FLASH_CLASS_BASE)
    CASE_CONTROL_ID(FLASH_CLASS)
    CASE_CONTROL_ID(FLASH_LED_MODE)
    CASE_CONTROL_ID(FLASH_STROBE_SOURCE)
    CASE_CONTROL_ID(FLASH_STROBE)
    CASE_CONTROL_ID(FLASH_STROBE_STOP)
    CASE_CONTROL_ID(FLASH_STROBE_STATUS)
    CASE_CONTROL_ID(FLASH_TIMEOUT)
    CASE_CONTROL_ID(FLASH_INTENSITY)
    CASE_CONTROL_ID(FLASH_TORCH_INTENSITY)
    CASE_CONTROL_ID(FLASH_INDICATOR_INTENSITY)
    CASE_CONTROL_ID(FLASH_FAULT)
    CASE_CONTROL_ID(FLASH_CHARGE)
    CASE_CONTROL_ID(FLASH_READY)
    CASE_CONTROL_ID(JPEG_CLASS_BASE)
    CASE_CONTROL_ID(JPEG_CLASS)
    CASE_CONTROL_ID(JPEG_CHROMA_SUBSAMPLING)
    CASE_CONTROL_ID(JPEG_RESTART_INTERVAL)
    CASE_CONTROL_ID(JPEG_COMPRESSION_QUALITY)
    CASE_CONTROL_ID(JPEG_ACTIVE_MARKER)
    CASE_CONTROL_ID(IMAGE_SOURCE_CLASS_BASE)
    CASE_CONTROL_ID(IMAGE_SOURCE_CLASS)
    CASE_CONTROL_ID(VBLANK)
    CASE_CONTROL_ID(HBLANK)
    CASE_CONTROL_ID(ANALOGUE_GAIN)
    CASE_CONTROL_ID(TEST_PATTERN_RED)
    CASE_CONTROL_ID(TEST_PATTERN_GREENR)
    CASE_CONTROL_ID(TEST_PATTERN_BLUE)
    CASE_CONTROL_ID(TEST_PATTERN_GREENB)
    CASE_CONTROL_ID(UNIT_CELL_SIZE)
    CASE_CONTROL_ID(NOTIFY_GAINS)
    CASE_CONTROL_ID(IMAGE_PROC_CLASS_BASE)
    CASE_CONTROL_ID(IMAGE_PROC_CLASS)
    CASE_CONTROL_ID(LINK_FREQ)
    CASE_CONTROL_ID(PIXEL_RATE)
    CASE_CONTROL_ID(TEST_PATTERN)
    CASE_CONTROL_ID(DEINTERLACING_MODE)
    CASE_CONTROL_ID(DIGITAL_GAIN)
    CASE_CONTROL_ID(DV_CLASS_BASE)
    CASE_CONTROL_ID(DV_CLASS)
    CASE_CONTROL_ID(DV_TX_HOTPLUG)
    CASE_CONTROL_ID(DV_TX_RXSENSE)
    CASE_CONTROL_ID(DV_TX_EDID_PRESENT)
    CASE_CONTROL_ID(DV_TX_MODE)
    CASE_CONTROL_ID(DV_TX_RGB_RANGE)
    CASE_CONTROL_ID(DV_TX_IT_CONTENT_TYPE)
    CASE_CONTROL_ID(DV_RX_POWER_PRESENT)
    CASE_CONTROL_ID(DV_RX_RGB_RANGE)
    CASE_CONTROL_ID(DV_RX_IT_CONTENT_TYPE)
    CASE_CONTROL_ID(FM_RX_CLASS_BASE)
    CASE_CONTROL_ID(FM_RX_CLASS)
    CASE_CONTROL_ID(TUNE_DEEMPHASIS)
    CASE_CONTROL_ID(RDS_RECEPTION)
    CASE_CONTROL_ID(RDS_RX_PTY)
    CASE_CONTROL_ID(RDS_RX_PS_NAME)
    CASE_CONTROL_ID(RDS_RX_RADIO_TEXT)
    CASE_CONTROL_ID(RDS_RX_TRAFFIC_ANNOUNCEMENT)
    CASE_CONTROL_ID(RDS_RX_TRAFFIC_PROGRAM)
    CASE_CONTROL_ID(RDS_RX_MUSIC_SPEECH)
    CASE_CONTROL_ID(RF_TUNER_CLASS_BASE)
    CASE_CONTROL_ID(RF_TUNER_CLASS)
    CASE_CONTROL_ID(RF_TUNER_BANDWIDTH_AUTO)
    CASE_CONTROL_ID(RF_TUNER_BANDWIDTH)
    CASE_CONTROL_ID(RF_TUNER_RF_GAIN)
    CASE_CONTROL_ID(RF_TUNER_LNA_GAIN_AUTO)
    CASE_CONTROL_ID(RF_TUNER_LNA_GAIN)
    CASE_CONTROL_ID(RF_TUNER_MIXER_GAIN_AUTO)
    CASE_CONTROL_ID(RF_TUNER_MIXER_GAIN)
    CASE_CONTROL_ID(RF_TUNER_IF_GAIN_AUTO)
    CASE_CONTROL_ID(RF_TUNER_IF_GAIN)
    CASE_CONTROL_ID(RF_TUNER_PLL_LOCK)
    CASE_CONTROL_ID(DETECT_CLASS_BASE)
    CASE_CONTROL_ID(DETECT_CLASS)
    CASE_CONTROL_ID(DETECT_MD_MODE)
    CASE_CONTROL_ID(DETECT_MD_GLOBAL_THRESHOLD)
    CASE_CONTROL_ID(DETECT_MD_THRESHOLD_GRID)
    CASE_CONTROL_ID(DETECT_MD_REGION_GRID)
    CASE_CONTROL_ID(CODEC_STATELESS_CLASS)
    CASE_CONTROL_ID(STATELESS_H264_DECODE_MODE) // V4L2_CID_CODEC_STATELESS_BASE
    CASE_CONTROL_ID(STATELESS_H264_START_CODE)
    CASE_CONTROL_ID(STATELESS_H264_SPS)
    CASE_CONTROL_ID(STATELESS_H264_PPS)
    CASE_CONTROL_ID(STATELESS_H264_SCALING_MATRIX)
    CASE_CONTROL_ID(STATELESS_H264_PRED_WEIGHTS)
    CASE_CONTROL_ID(STATELESS_H264_SLICE_PARAMS)
    CASE_CONTROL_ID(STATELESS_H264_DECODE_PARAMS)
    CASE_CONTROL_ID(STATELESS_FWHT_PARAMS)
    CASE_CONTROL_ID(STATELESS_VP8_FRAME)
    CASE_CONTROL_ID(STATELESS_MPEG2_SEQUENCE)
    CASE_CONTROL_ID(STATELESS_MPEG2_PICTURE)
    CASE_CONTROL_ID(STATELESS_MPEG2_QUANTISATION)
    CASE_CONTROL_ID(STATELESS_HEVC_SPS)
    CASE_CONTROL_ID(STATELESS_HEVC_PPS)
    CASE_CONTROL_ID(STATELESS_HEVC_SLICE_PARAMS)
    CASE_CONTROL_ID(STATELESS_HEVC_SCALING_MATRIX)
    CASE_CONTROL_ID(STATELESS_HEVC_DECODE_PARAMS)
    CASE_CONTROL_ID(STATELESS_HEVC_DECODE_MODE)
    CASE_CONTROL_ID(STATELESS_HEVC_START_CODE)
    CASE_CONTROL_ID(STATELESS_HEVC_ENTRY_POINT_OFFSETS)
    CASE_CONTROL_ID(COLORIMETRY_CLASS)
    CASE_CONTROL_ID(COLORIMETRY_HDR10_CLL_INFO) // V4L2_CID_COLORIMETRY_CLASS_BASE
    CASE_CONTROL_ID(COLORIMETRY_HDR10_MASTERING_DISPLAY)
    CASE_CONTROL_ID(STATELESS_VP9_FRAME)
    CASE_CONTROL_ID(STATELESS_VP9_COMPRESSED_HDR)
    CASE_CONTROL_ID(STATELESS_AV1_SEQUENCE)
    CASE_CONTROL_ID(STATELESS_AV1_TILE_GROUP_ENTRY)
    CASE_CONTROL_ID(STATELESS_AV1_FRAME)
    CASE_CONTROL_ID(STATELESS_AV1_FILM_GRAIN)
    CASE_CONTROL_ID(PRIVATE_BASE)
    default:
        return nullptr;
    }
#undef CASE_CONTROL_ID
}

std::string v4l2_control_id_to_str(int id)
{
    const char * const n = v4l2_control_id_to_c_str(id);
    const char * base;
    int diff;
    if (id > V4L2_CID_PRIVATE_BASE) {
        base = "PRIVATE_BASE";
        diff = id - V4L2_CID_PRIVATE_BASE;
    } else {
        base = "BASE";
        diff = id - V4L2_CID_BASE;
    }
    return std::format("{} (#{}, {}+{})", (n ? n : "Unknown"), id, base, diff);
}

int f_G_CTRL(void * a)
{
    v4l2_control & control = *static_cast<v4l2_control*>(a);
     if (control.id == V4L2_CID_MIN_BUFFERS_FOR_OUTPUT) {
        std::print("[EMU] In G_CTRL: id=MIN_BUFFERS_FOR_OUTPUT: 1.\n");
         control.value = 1;
         return 0;
     }
    std::print("[EMU] In G_CTRL: id={}, value={} - Ignored.\n", v4l2_control_id_to_str(control.id), control.value);
    errno = EINVAL;
    return -1;
}

int f_S_CTRL(void * a)
{
    v4l2_control & control = *static_cast<v4l2_control*>(a);
    std::print("[EMU] In S_CTRL: id={}, value={} - Ignored.\n", v4l2_control_id_to_str(control.id), control.value);
    errno = EINVAL;
    return -1;
}

int f_G_PARM(void * a)
{
    v4l2_streamparm & param = *static_cast<v4l2_streamparm*>(a);
    std::print("[EMU] In G_PARM: type={}\n", v4l2_buf_type_to_str(param.type));
    // we expose only one output format for this output
    if (param.type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
        errno = EINVAL;
        return -1;
    }
    v4l2_outputparm & out_param(param.parm.output);
    out_param.capability = 0;
    out_param.outputmode = 0;
    out_param.timeperframe = EMULATED_FPS;
    out_param.extendedmode = 0;
    out_param.writebuffers = 2;
    return 0;
}

std::string v4l2_selection_target_to_str(int t)
{
#define CASE_SELECTION_TARGET(n) case V4L2_SEL_TGT_ ## n: return #n;
    switch(t) {
    CASE_SELECTION_TARGET(CROP)
    CASE_SELECTION_TARGET(CROP_DEFAULT)
    CASE_SELECTION_TARGET(CROP_BOUNDS)
    CASE_SELECTION_TARGET(NATIVE_SIZE)
    CASE_SELECTION_TARGET(COMPOSE)
    CASE_SELECTION_TARGET(COMPOSE_DEFAULT)
    CASE_SELECTION_TARGET(COMPOSE_BOUNDS)
    CASE_SELECTION_TARGET(COMPOSE_PADDED)
    default:
        return std::format("Unknown #{}", int(t));
    }
#undef CASE_SELECTION_TARGET
}

int f_G_SELECTION(void * a)
{
    v4l2_selection & selection = *static_cast<v4l2_selection*>(a);
    std::print("[EMU] In G_SELECTION: type={}, target={}\n",
        v4l2_buf_type_to_str(selection.type), v4l2_selection_target_to_str(selection.target));
    // we expose only one output format for this output
    if (selection.type != V4L2_BUF_TYPE_VIDEO_OUTPUT or
            (selection.target != V4L2_SEL_TGT_CROP and
                selection.target != V4L2_SEL_TGT_CROP_DEFAULT and
                selection.target != V4L2_SEL_TGT_COMPOSE_DEFAULT and
                selection.target != V4L2_SEL_TGT_COMPOSE)) {
        errno = EINVAL;
        return -1;
    }
    selection.flags = 0;
    constexpr v4l2_rect rect{0, 0, EMULATED_WIDTH, EMULATED_HEIGHT};
    selection.r = rect;
    return 0;
}

int f_S_SELECTION(void * a)
{
    v4l2_selection & selection = *static_cast<v4l2_selection*>(a);
    std::print("[EMU] In S_SELECTION: type={}, target={}\n",
        v4l2_buf_type_to_str(selection.type), v4l2_selection_target_to_str(selection.target));
    // we expose only one output format for this output
    if (selection.type != V4L2_BUF_TYPE_VIDEO_OUTPUT or
            (selection.target != V4L2_SEL_TGT_CROP and
                selection.target != V4L2_SEL_TGT_CROP_DEFAULT and
                selection.target != V4L2_SEL_TGT_COMPOSE_DEFAULT and
                selection.target != V4L2_SEL_TGT_COMPOSE)) {
        errno = EINVAL;
        return -1;
    }
#define CHECK_SELECTION_FIELD(n, v) \
    if (selection.n != v) { \
        std::print("[EMU] S_SELECTION, field '{}': expected {}, got{}\n", #n, v, selection.flags); \
        errno = EINVAL; \
        return -1; \
    }
    CHECK_SELECTION_FIELD(flags, 0)
    CHECK_SELECTION_FIELD(r.left, 0)
    CHECK_SELECTION_FIELD(r.top, 0)
    CHECK_SELECTION_FIELD(r.width, EMULATED_WIDTH)
    CHECK_SELECTION_FIELD(r.height, EMULATED_HEIGHT)
#undef CHECK_SELECTION_FIELD
    return 0;
}

template<typename T>
std::string v4l2_memory_to_str(const T t)
{
#define CASE_V4L2_MEMORY(n) case V4L2_MEMORY_ ## n: return #n;
    switch(static_cast<v4l2_memory>(t)) {
    CASE_V4L2_MEMORY(MMAP)
    CASE_V4L2_MEMORY(USERPTR)
    CASE_V4L2_MEMORY(OVERLAY)
    CASE_V4L2_MEMORY(DMABUF)
    default:
        return std::format("Unknown #{}", int(t));
    }
#undef CASE_V4L2_MEMORY
}

int f_REQBUFS(void * a)
{
    v4l2_requestbuffers & request = *static_cast<v4l2_requestbuffers*>(a);
    std::print("[EMU] In REQBUFS: count={}, type={}, memory={}, capabilities={}, flags={}\n", request.count,
            v4l2_buf_type_to_str(request.type), v4l2_memory_to_str(request.memory), request.capabilities, request.flags);
#define CHECK_MEMBER(f, v) \
    if (request.f != v) { \
        std::print("[EMU] Check failed for REQBUFS, {}: expected {}, got {}\n", \
            #f, int(v), int(request.f)); \
        errno = EINVAL; \
        return -1; \
    }
    CHECK_MEMBER(type, V4L2_BUF_TYPE_VIDEO_OUTPUT)
    CHECK_MEMBER(memory, V4L2_MEMORY_MMAP)
#undef CHECK_MEMBER
    if (!request.count or request.count > EMULATED_BUFFER_MMAP_MAX) 
        request.count = EMULATED_BUFFER_MMAP_MAX;
    request.capabilities = V4L2_BUF_CAP_SUPPORTS_MMAP;
    request.flags = 0;
    return 0;
}

/* we don't need it for minimal implemenation
int f_CREATE_BUFS(void * a)
{
    v4l2_create_buffers & buffers = *static_cast<v4l2_create_buffers*>(a);
    std::print("[EMU] In CREATE_BUFS: index={}, count={}, type={}, memory={}, capabilities={}, flags={}, max_num_buffers={}\n", buffers.count,
            buffers.index, v4l2_buf_type_to_str(buffers.format.type), v4l2_memory_to_str(buffers.memory), buffers.capabilities, buffers.flags, buffers.max_num_buffers);
#define CHECK_MEMBER(f, v) \
    if (buffers.f != v) { \
        std::print("[EMU] Check failed for CREATE_BUFS, {}: expected {}, got {}\n", \
            #f, int(v), int(buffers.f)); \
        errno = EINVAL; \
        return -1; \
    }
    CHECK_MEMBER(format.type, V4L2_BUF_TYPE_VIDEO_OUTPUT)
    v4l2_pix_format & pix_fmt = buffers.format.fmt.pix;
#define CHECK_PIXEL_FORMAT_STRUCT_MEMBER(f) \
    if (pix_fmt.f != EMULATED_PIXEL_FORMAT_STRUCT.f) { \
        std::print("[EMU] Check failed for CREATE_BUFS, {}: expected {}, got {}\n", \
            #f, EMULATED_PIXEL_FORMAT_STRUCT.f, pix_fmt.f); \
        errno = EINVAL; \
        return -1; \
    }
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(width);
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(height);
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(pixelformat);
    CHECK_PIXEL_FORMAT_STRUCT_MEMBER(field);
    pix_fmt = EMULATED_PIXEL_FORMAT_STRUCT;
#undef CHECK_PIXEL_FORMAT_STRUCT_MEMBER
    CHECK_MEMBER(memory, V4L2_MEMORY_MMAP)
#undef CHECK_MEMBER
    buffers.index = 0;
    if (!buffers.count or buffers.count > EMULATED_BUFFER_MMAP_MAX) 
        buffers.count = EMULATED_BUFFER_MMAP_MAX;
    buffers.capabilities = V4L2_BUF_CAP_SUPPORTS_MMAP | V4L2_BUF_CAP_SUPPORTS_MAX_NUM_BUFFERS;
    buffers.flags = 0;
    buffers.max_num_buffers = EMULATED_BUFFER_MMAP_MAX;
    return 0;
}
*/

int f_QUERYBUF(void * a)
{
    v4l2_buffer & buffer = *static_cast<v4l2_buffer*>(a);
    std::print("[EMU] In QUERYBUF: index={}, type={}, bytesused={}, flags={}, buffers_used={}\n",
        buffer.index, v4l2_buf_type_to_str(buffer.type), buffer.bytesused, buffer.flags, buffers_used);
    if (buffers_used == EMULATED_BUFFER_MMAP_MAX) {
        std::print("[EMU] Check failed for QUERYBUF: no more buffers\n");
        errno = EINVAL;
        return -1;
    }
#define CHECK_MEMBER(f, v) \
    if (buffer.f != v) { \
        std::print("[EMU] Check failed for QUERYBUF, {}: expected {}, got {}\n", \
            #f, int(v), int(buffer.f)); \
        errno = EINVAL; \
        return -1; \
    }
    CHECK_MEMBER(index, buffers_used)
    CHECK_MEMBER(type, V4L2_BUF_TYPE_VIDEO_OUTPUT)
#undef CHECK_MEMBER
    buffer.bytesused = 0; // application must set this field when it sends a buffer to the driver
    buffer.flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC; // V4L2_BUF_FLAG_MAPPED;
    buffer.field = V4L2_FIELD_ANY; // V4L2_FIELD_NONE; // progressive
    memset(&buffer.timestamp, 0, sizeof buffer.timestamp);
    memset(&buffer.timecode, 0, sizeof buffer.timecode);
    buffer.sequence = 0;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.m.offset = EMULATED_BUFFER_MMAP_OFFSETS[buffers_used];
    buffer.length = EMULATED_PIXEL_FORMAT_STRUCT.sizeimage;
    ++buffers_used;
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
    CASE_REQ_ARG_STUB(CREATE_BUFS) // CASE_REQ_ARG
    CASE_REQ_ARG(CROPCAP)
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
    CASE_REQ_ARG(ENUM_FMT)
    CASE_REQ_ARG(ENUM_FRAMESIZES)
    CASE_REQ_ARG(ENUM_FRAMEINTERVALS)
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
    CASE_REQ_ARG(G_CTRL)
    CASE_REQ_ARG(S_CTRL)
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
    CASE_REQ_ARG(G_FMT)
    CASE_REQ_ARG(S_FMT)
    CASE_REQ_ARG(TRY_FMT)
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
    CASE_REQ_ARG(G_PARM)
    CASE_REQ_ARG_STUB(S_PARM)
    CASE_REQ_ARG_STUB(G_PRIORITY)
    CASE_REQ_ARG_STUB(S_PRIORITY)
    CASE_REQ_ARG(G_SELECTION)
    CASE_REQ_ARG(S_SELECTION)
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
    CASE_REQ_ARG(QUERYBUF)
    CASE_REQ_ARG(QUERYCTRL)
    CASE_REQ_ARG_STUB(QUERY_EXT_CTRL)
    CASE_REQ_ARG_STUB(QUERYMENU)
    CASE_REQ_ARG_STUB(QUERY_DV_TIMINGS)
    CASE_REQ_ARG_STUB(QUERYSTD)
    // CASE_REQ_ARG_STUB(SUBDEV_QUERYSTD)
    CASE_REQ_ARG(REQBUFS)
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
