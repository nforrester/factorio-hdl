def fdlt_tests():
    fdlts = native.glob(["**/*.fdlt"])
    fdls = native.glob(["**/*.fdl"])
    scms = native.glob(["**/*.scm"])
    for fdlt in fdlts:
        name = fdlt[:-5]
        native.sh_test(name = name,
                       srcs = ["//src:circuit_tester"],
                       data = [fdlt] + fdls + scms,
                       args = ["$(location " + fdlt + ")"])
