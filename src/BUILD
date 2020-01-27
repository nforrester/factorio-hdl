load("util.bzl", "bins", "gtests", "common_copts")

cc_library(
    name = "factorio_lib",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"], exclude = ["**/*_bin.cpp", "**/*_gtest.cpp"]),
    copts = common_copts,
    linkopts = ['-lz'],
)

bins(deps=[":factorio_lib"])
gtests(deps=[":factorio_lib"])
