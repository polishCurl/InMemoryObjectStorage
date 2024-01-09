cc_test(
    name = "integration_tests",
    srcs = ["integration_tests.cpp"],
    data = glob(["data/*"]),
    deps = [
        "//server/object_storage",
        "@googletest//:gtest_main",
    ],
)
