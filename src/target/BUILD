load("toolchain_config.bzl", "rv32i_freestanding_cc_toolchain_config")

constraint_value(
    name = "riscv-rv32i",
    constraint_setting = "@platforms//cpu:cpu",
)

platform(
    name = "factorio-riscv",
    constraint_values = [
        ":riscv-rv32i",
        "@platforms//os:none",
    ],
)

rv32i_freestanding_cc_toolchain_config(
    name = "rv32i-freestanding-toolchain-config",
)

filegroup(
    name = "rv32i-freestanding-toolchain-files",
)

cc_toolchain(
    name = "rv32i-freestanding-toolchain-definition",
    toolchain_config = ":rv32i-freestanding-toolchain-config",
    all_files = ":rv32i-freestanding-toolchain-files",
    compiler_files = ":rv32i-freestanding-toolchain-files",
    strip_files = ":rv32i-freestanding-toolchain-files",
    objcopy_files = ":rv32i-freestanding-toolchain-files",
    linker_files = ":rv32i-freestanding-toolchain-files",
    dwp_files = ":rv32i-freestanding-toolchain-files",
)

toolchain(
    name = "rv32i-freestanding-toolchain",
    toolchain = ":rv32i-freestanding-toolchain-definition",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
    target_compatible_with = [
        ":riscv-rv32i",
        "@platforms//os:none",
    ],
)

common_copts_base = ["-Wall", "-Werror", "-g", "-ffreestanding", "-fpic"]
common_copts = ["-std=c17"] + common_copts_base
common_cppopts = ["-std=c++17"] + common_copts_base
common_linkopts = ["-static", "-nostdlib", "-nostdinc", "-lgcc"]

cc_binary(
    name = "mult",
    srcs = ["mult.c"],
    copts = common_copts,
    linkopts = common_linkopts,
)

cc_binary(
    name = "memory-test",
    srcs = ["memory-test.cpp"],
    copts = common_cppopts,
    linkopts = common_linkopts,
    visibility = ["//visibility:public"],
)
