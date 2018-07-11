load("//build/bazel:qt.bzl", "qt5_package")
load("//build/bazel:pkg_config.bzl", "pkg_config_package")
load("//build/bazel:homebrew.bzl", "homebrew_repository")

pkg_config_package(name = "flac", modname = "flac")
pkg_config_package(name = "libchromaprint", modname = "libchromaprint")
pkg_config_package(name = "libmodplug", modname = "libmodplug")
pkg_config_package(name = "lilv-0", modname = "lilv-0")
pkg_config_package(name = "mad", modname = "mad")
pkg_config_package(name = "protobuf-lite", modname = "protobuf-lite")
pkg_config_package(name = "ogg", modname = "ogg")
pkg_config_package(name = "opus", modname = "opus")
pkg_config_package(name = "opusfile", modname = "opusfile")
pkg_config_package(name = "portaudio", modname = "portaudio-2.0")
pkg_config_package(name = "taglib", modname = "taglib")
pkg_config_package(name = "rubberband", modname = "rubberband")
pkg_config_package(name = "vorbis", modname = "vorbis")
pkg_config_package(name = "vorbisenc", modname = "vorbisenc")
pkg_config_package(name = "vorbisfile", modname = "vorbisfile")
pkg_config_package(name = "sndfile", modname = "sndfile")
pkg_config_package(name = "upower-glib", modname = "upower-glib")

homebrew_repository(
    name = "portmidi",
    build_file = "//build/bazel:BUILD.portmidi",
)

homebrew_repository(
    name = "protobuf",
    package_name = "com_google_protobuf",
    build_file = "//build/bazel:BUILD.protobuf",
)

qt5_package()
