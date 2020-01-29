common_copts = ["-Wall", "-Werror", "-std=c++2a"]

def bins(deps):
    bin_srcs = native.glob(["**/*_bin.cpp"])
    for src in bin_srcs:
        native.cc_binary(name = src[:-8],
                         srcs = [src],
                         deps = deps,
                         copts = common_copts)

def gtests(deps):
    test_srcs = native.glob(["**/*_gtest.cpp"])
    for src in test_srcs:
        native.cc_test(name = src[:-4],
                       srcs = [src],
                       deps = deps + ["@gtest//:main"],
                       copts = common_copts + ["-Iexternal/gtest/googletest/include"])
