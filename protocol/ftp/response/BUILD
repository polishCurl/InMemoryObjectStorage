cc_library(
    name = "ftp_response",
    hdrs = [
        "src/ftp_response.hpp",
    ],
    visibility = ["//server/object_storage:__subpackages__"],
    deps = ["//protocol:serialize_interface"],
)

cc_test(
    name = "ftp_response_test",
    srcs = ["test/ftp_response_test.cpp"],
    deps = [
        ":ftp_response",
        "@googletest//:gtest_main",
    ],
)
