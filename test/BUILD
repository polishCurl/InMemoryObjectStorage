cc_test(
    name = "http_integration_tests",
    srcs = [
        "http_integration_tests.cpp",
        "integration_tests.hpp",
    ],
    data = glob(["data/*"]),
    tags = ["exclusive"],
    deps = [
        "//server/object_storage",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "ftp_integration_tests",
    srcs = [
        "ftp_integration_tests.cpp",
        "integration_tests.hpp",
    ],
    data = glob(["data/*"]),
    tags = ["exclusive"],
    deps = [
        "//server/object_storage",
        "@googletest//:gtest_main",
    ],
)

test_suite(
    name = "integration_tests",
    tests = [
        ":ftp_integration_tests",
        ":http_integration_tests",
    ],
)
