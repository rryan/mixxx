load("//build/bazel:qt.bzl", "qt_moc_header")

cc_library(
    name = "fidlib",
    hdrs = ["fidlib/fidlib.h"],
    defines = ['T_LINUX'],
    srcs = [
        "fidlib/fidlib.c",
        "fidlib/fidmkf.h",
        "fidlib/fidrf_cmdlist.h",
    ],
    includes = ["fidlib"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "hidapi",
    hdrs = [
        "hidapi-0.8.0-rc1/hidapi/hidapi.h",
    ],
    srcs = [
        "hidapi-0.8.0-rc1/mac/hid.c",
    ],
    linkopts = ["-framework IOKit",
                "-framework CoreFoundation"],
    includes = ["hidapi-0.8.0-rc1/hidapi"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "replaygain",
    hdrs = ["replaygain/replaygain.h"],
    srcs = ["replaygain/replaygain.cpp"],
    includes = ["replaygain"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ebur128",
    hdrs = ["libebur128/ebur128/ebur128.h"],
    srcs = ["libebur128/ebur128/ebur128.c"],
    includes = [
        "libebur128/ebur128",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "soundtouch",
    hdrs = [
        "soundtouch-2.0.0/SoundTouch.h",
    ],
    srcs = [
        "soundtouch-2.0.0/AAFilter.cpp",
        "soundtouch-2.0.0/AAFilter.h",
        "soundtouch-2.0.0/BPMDetect.cpp",
        "soundtouch-2.0.0/BPMDetect.h",
        "soundtouch-2.0.0/FIFOSampleBuffer.cpp",
        "soundtouch-2.0.0/FIFOSampleBuffer.h",
        "soundtouch-2.0.0/FIFOSamplePipe.h",
        "soundtouch-2.0.0/FIRFilter.cpp",
        "soundtouch-2.0.0/FIRFilter.h",
        "soundtouch-2.0.0/InterpolateCubic.cpp",
        "soundtouch-2.0.0/InterpolateCubic.h",
        "soundtouch-2.0.0/InterpolateLinear.cpp",
        "soundtouch-2.0.0/InterpolateLinear.h",
        "soundtouch-2.0.0/InterpolateShannon.cpp",
        "soundtouch-2.0.0/InterpolateShannon.h",
        "soundtouch-2.0.0/PeakFinder.cpp",
        "soundtouch-2.0.0/PeakFinder.h",
        "soundtouch-2.0.0/RateTransposer.cpp",
        "soundtouch-2.0.0/RateTransposer.h",
        "soundtouch-2.0.0/SoundTouch.cpp",
        "soundtouch-2.0.0/STTypes.h",
        "soundtouch-2.0.0/TDStretch.cpp",
        "soundtouch-2.0.0/TDStretch.h",
        "soundtouch-2.0.0/cpu_detect.h",
        "soundtouch-2.0.0/soundtouch_config.h",
        "soundtouch-2.0.0/cpu_detect_x86.cpp",
        "soundtouch-2.0.0/mmx_optimized.cpp",
        "soundtouch-2.0.0/sse_optimized.cpp",
    ],
    includes = ["soundtouch-2.0.0"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "qtscript-bytearray",
    hdrs = [
        "qtscript-bytearray/bytearrayclass.h",
        "qtscript-bytearray/bytearrayprototype.h",
    ],
    srcs = [
        qt_moc_header("qtscript-bytearray/bytearrayclass.h"),
        qt_moc_header("qtscript-bytearray/bytearrayprototype.h"),
        "qtscript-bytearray/bytearrayclass.cpp",
        "qtscript-bytearray/bytearrayprototype.cpp",
    ],
    deps = [
        "@qt5//:headers",
        #"@Qt5Core//:lib",
    ],
    includes = [".", "qtscript-bytearray"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "pa_ringbuffer",
    hdrs = [
        "portaudio/pa_ringbuffer.h",
    ],
    srcs = [
        "portaudio/pa_ringbuffer.c",
        "portaudio/pa_memorybarrier.h",
    ],
    includes = ["portaudio"],
    visibility = ["//visibility:public"],
)


cc_library(
    name = "reverb",
    hdrs = [
        "reverb/Reverb.h",
    ],
    srcs = [
        "reverb/dsp/Delay.h",
        "reverb/dsp/FPTruncateMode.h",
        "reverb/dsp/IIR1.h",
        "reverb/dsp/Sine.h",
        "reverb/dsp/util.h",
        "reverb/Reverb.cc",
        "reverb/basics.h",
    ],
    deps = [
        #"@Qt5Core//:lib",
        "@qt5//:Qt5Core",
    ],
    includes = ["reverb"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gtest_prod",
    hdrs = [
        "gtest-1.7.0/include/gtest/gtest_prod.h"
    ],
    includes = ["gtest-1.7.0/include"],
    visibility = ["//visibility:public"],
)
