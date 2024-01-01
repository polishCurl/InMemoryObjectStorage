cc_library(
    name = "http_parser_interface",
    srcs = [
    ],
    hdrs = [
        "ihttp_parser.hpp",
    ],
    strip_include_prefix = "",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "http_parser",
    srcs = [
        "src/http_parser.cpp",
    ],
    hdrs = [
        "src/http_parser.hpp",
    ],
    strip_include_prefix = "",
    deps = [":http_parser_interface"],
)

cc_test(
    name = "http_parser_test",
    srcs = ["test/http_parser_test.cpp"],
    deps = [
        ":http_parser",
        "@googletest//:gtest_main",
    ],
)
