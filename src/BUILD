common_copts = ["-g", "-Wall", "-Werror", "-std=c++2a"]

cc_library(
    name = "factorio_lib",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"], exclude = ["**/*_bin.cpp"]),
    copts = common_copts,
)

cc_binary(
    name = "factorio",
    srcs = ["factorio_bin.cpp"],
    deps = [":factorio_lib"],
    copts = common_copts,
)
