cc_library(
    name = "http_response",
    srcs = [
        "src/http_response.cpp",
    ],
    hdrs = [
        "src/http_response.hpp",
    ],
    visibility = ["//server/object_storage:__subpackages__"],
    deps = ["//protocol:serialize_interface"],
)

cc_test(
    name = "http_response_test",
    srcs = ["test/http_response_test.cpp"],
    deps = [
        ":http_response",
        "@googletest//:gtest_main",
    ],
)
