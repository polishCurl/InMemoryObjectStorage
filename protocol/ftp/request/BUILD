cc_library(
    name = "ftp_parser",
    srcs = [
        "src/ftp_parser.cpp",
    ],
    hdrs = [
        "src/ftp_parser.hpp",
    ],
    strip_include_prefix = "",
    deps = ["//utils"],
)

cc_test(
    name = "ftp_parser_test",
    srcs = ["test/ftp_parser_test.cpp"],
    deps = [
        ":ftp_parser",
        "@googletest//:gtest_main",
    ],
)
