package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "%{name}",
    hdrs = glob(["include/**"]),
    defines = ["%{defines}"],
    includes = ["%{includes}"],
    linkopts = ["%{linkopts}"],
)