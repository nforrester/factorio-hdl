common_copts = ["-Wall", "-Werror", "-std=c++2a"]

def bins(deps = []):
    bin_srcs = native.glob(["**/*_bin.cpp"])
    for src in bin_srcs:
        name = src[:-8]
        native.cc_binary(name = name + "_host",
                         srcs = [src],
                         deps = deps,
                         copts = common_copts,
                         tags = ["manual"])
        native.genrule(name = "build_" + name,
                       srcs = [],
                       outs = [name],
                       cmd = "cp $(location :" + name + "_host) $@",
                       tools = [":" + name + "_host"],
                       visibility = ["//visibility:public"])

def gtests(deps = [], extra_data = dict()):
    test_srcs = native.glob(["**/*_gtest.cpp"])
    for src in test_srcs:
        name = src[:-4]
        bin_name = name + "_bin"
        data = dict()
        if name in extra_data.keys():
            data = extra_data[name]
        native.cc_binary(name = bin_name + "_host",
                         srcs = [src],
                         deps = deps + ["@gtest//:main"],
                         data = data,
                         copts = common_copts + ["-Iexternal/gtest/googletest/include"],
                         tags = ["manual"])
        native.genrule(name = "build_" + bin_name,
                       srcs = [],
                       outs = [bin_name],
                       cmd = "cp $(location :" + bin_name + "_host) $@",
                       tools = [":" + bin_name + "_host"],
                       visibility = ["//visibility:public"])
        native.sh_test(name = name,
                       srcs = [":" + bin_name],
                       data = data)
