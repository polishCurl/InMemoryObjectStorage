cc_library(
    name = "http_parser",
    srcs = [
        "src/http_parser.cpp",
    ],
    hdrs = [
        "src/http_parser.hpp",
    ],
    deps = [
        "//protocol:member_access_interface",
        "//protocol:validate_interface",
        "//utils",
    ],
)

cc_test(
    name = "http_parser_test",
    srcs = ["test/http_parser_test.cpp"],
    deps = [
        ":http_parser",
        "@googletest//:gtest_main",
    ],
)
