common_copts = ["-Wall", "-Werror", "-std=c++2a"]

cc_library(
    name = "factorio_lib",
    hdrs = glob(["*.h"]),
    copts = common_copts,
)

cc_binary(
    name = "factorio",
    srcs = ["factorio.cpp"],
    deps = [":factorio_lib"],
    copts = common_copts,
)
