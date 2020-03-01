def fdlt_tests(extra_data = dict()):
    fdlts = native.glob(["**/*.fdlt"])
    fdls = native.glob(["**/*.fdl"])
    scms = native.glob(["**/*.scm"])
    for fdlt in fdlts:
        name = fdlt[:-5]
        data = []
        if name in extra_data.keys():
            data = extra_data[name]
        native.sh_test(name = name,
                       srcs = ["//src:circuit_tester"],
                       data = data + [fdlt] + fdls + scms,
                       args = ["$(location " + fdlt + ")"])
