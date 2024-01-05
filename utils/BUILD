cc_library(
    name = "utils",
    srcs = [
        "src/utils.cpp",
    ],
    hdrs = [
        "src/utils.hpp",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "utils_test",
    srcs = ["test/utils_test.cpp"],
    deps = [
        ":utils",
        "@googletest//:gtest_main",
    ],
)
