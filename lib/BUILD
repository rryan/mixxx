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
    ],
    includes = [
        ".",
        "qtscript-bytearray"
    ],
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

cc_library(
    name = "gtest",
    hdrs = [
        "gtest-1.7.0/include/gtest/gtest-death-test.h",
        "gtest-1.7.0/include/gtest/gtest-message.h",
        "gtest-1.7.0/include/gtest/gtest-param-test.h",
        "gtest-1.7.0/include/gtest/gtest-printers.h",
        "gtest-1.7.0/include/gtest/gtest-spi.h",
        "gtest-1.7.0/include/gtest/gtest-test-part.h",
        "gtest-1.7.0/include/gtest/gtest-typed-test.h",
        "gtest-1.7.0/include/gtest/gtest.h",
        "gtest-1.7.0/include/gtest/gtest_pred_impl.h",
        "gtest-1.7.0/include/gtest/gtest_prod.h",
    ],
    srcs = [
        "gtest-1.7.0/include/gtest/internal/gtest-death-test-internal.h",
        "gtest-1.7.0/include/gtest/internal/gtest-filepath.h",
        "gtest-1.7.0/include/gtest/internal/gtest-internal.h",
        "gtest-1.7.0/include/gtest/internal/gtest-linked_ptr.h",
        "gtest-1.7.0/include/gtest/internal/gtest-param-util-generated.h",
        "gtest-1.7.0/include/gtest/internal/gtest-param-util.h",
        "gtest-1.7.0/include/gtest/internal/gtest-port.h",
        "gtest-1.7.0/include/gtest/internal/gtest-string.h",
        "gtest-1.7.0/include/gtest/internal/gtest-tuple.h",
        "gtest-1.7.0/include/gtest/internal/gtest-type-util.h",
        "gtest-1.7.0/src/gtest.cc",
        "gtest-1.7.0/src/gtest-death-test.cc",
        "gtest-1.7.0/src/gtest-filepath.cc",
        "gtest-1.7.0/src/gtest-internal-inl.h",
        "gtest-1.7.0/src/gtest-port.cc",
        "gtest-1.7.0/src/gtest-printers.cc",
        "gtest-1.7.0/src/gtest-test-part.cc",
        "gtest-1.7.0/src/gtest-typed-test.cc",
    ],
    includes = [
        "gtest-1.7.0",
        "gtest-1.7.0/include"
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gmock",
    hdrs = [
        "gmock-1.7.0/include/gmock/gmock-actions.h",
        "gmock-1.7.0/include/gmock/gmock-cardinalities.h",
        "gmock-1.7.0/include/gmock/gmock-generated-actions.h",
        "gmock-1.7.0/include/gmock/gmock-generated-function-mockers.h",
        "gmock-1.7.0/include/gmock/gmock-generated-matchers.h",
        "gmock-1.7.0/include/gmock/gmock-generated-nice-strict.h",
        "gmock-1.7.0/include/gmock/gmock-matchers.h",
        "gmock-1.7.0/include/gmock/gmock-more-actions.h",
        "gmock-1.7.0/include/gmock/gmock-more-matchers.h",
        "gmock-1.7.0/include/gmock/gmock-spec-builders.h",
        "gmock-1.7.0/include/gmock/gmock.h",
    ],
    srcs = [
        "gmock-1.7.0/include/gmock/internal/gmock-generated-internal-utils.h",
        "gmock-1.7.0/include/gmock/internal/gmock-internal-utils.h",
        "gmock-1.7.0/include/gmock/internal/gmock-port.h",
        "gmock-1.7.0/src/gmock-cardinalities.cc",
        "gmock-1.7.0/src/gmock-internal-utils.cc",
        "gmock-1.7.0/src/gmock-matchers.cc",
        "gmock-1.7.0/src/gmock-spec-builders.cc",
        "gmock-1.7.0/src/gmock.cc",
    ],
    deps = [
        ":gtest",
    ],
    includes = [
        "gmock-1.7.0/include"
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "benchmark",
    hdrs = [
        "benchmark/include/benchmark/benchmark.h",
        "benchmark/include/benchmark/benchmark_api.h",
        "benchmark/include/benchmark/macros.h",
        "benchmark/include/benchmark/reporter.h",
    ],
    srcs = [
        "benchmark/src/arraysize.h",
        "benchmark/src/benchmark.cc",
        "benchmark/src/check.h",
        "benchmark/src/colorprint.cc",
        "benchmark/src/colorprint.h",
        "benchmark/src/commandlineflags.cc",
        "benchmark/src/commandlineflags.h",
        "benchmark/src/console_reporter.cc",
        "benchmark/src/csv_reporter.cc",
        "benchmark/src/cycleclock.h",
        "benchmark/src/internal_macros.h",
        "benchmark/src/json_reporter.cc",
        "benchmark/src/log.cc",
        "benchmark/src/log.h",
        "benchmark/src/mutex.h",
        "benchmark/src/re.h",
        "benchmark/src/re_std.cc",
        "benchmark/src/reporter.cc",
        "benchmark/src/sleep.cc",
        "benchmark/src/sleep.h",
        "benchmark/src/stat.h",
        "benchmark/src/string_util.cc",
        "benchmark/src/string_util.h",
        "benchmark/src/sysinfo.cc",
        "benchmark/src/sysinfo.h",
        "benchmark/src/walltime.cc",
        "benchmark/src/walltime.h",
    ],
    defines = [
        "HAVE_STD_REGEX",
        "HAVE_STEADY_CLOCK",
        "HAVE_THREAD_SAFETY_ATTRIBUTES",  # TODO(rjryan): Clang only.
    ],
    includes = ["benchmark/include"],
    visibility = ["//visibility:public"],
)
