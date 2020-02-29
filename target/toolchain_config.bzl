load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl", "tool_path")

def _rv32i_freestanding_cc_toolchain_impl(ctx):
    return [
        cc_common.create_cc_toolchain_config_info(
            ctx = ctx,
            toolchain_identifier = "riscv32-none-elf",
            host_system_name = "x86_64-pc-linux-gnu",
            target_system_name = "riscv32-none-elf",
            target_cpu = "rv32i",
            target_libc = "none",
            compiler = "gcc-9.2.0",
            abi_version = "gcc-9.2",
            abi_libc_version = "none",
            tool_paths = [
                tool_path(
                    name = "ar",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-ar",
                ),
                tool_path(
                    name = "cpp",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-cpp",
                ),
                tool_path(
                    name = "gcc",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-gcc",
                ),
                tool_path(
                    name = "gcov",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-gcov",
                ),
                tool_path(
                    name = "ld",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-ld",
                ),
                tool_path(
                    name = "nm",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-nm",
                ),
                tool_path(
                    name = "objdump",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-objdump",
                ),
                tool_path(
                    name = "strip",
                    path = "/home/neil/packages/riscv-toolchain/install/bin/riscv32-none-elf-strip",
                ),
            ],
        ),
    ]

rv32i_freestanding_cc_toolchain_config = rule(
    implementation = _rv32i_freestanding_cc_toolchain_impl,
    provides = [CcToolchainConfigInfo],
)
