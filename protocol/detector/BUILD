cc_library(
    name = "protocol_detector",
    srcs = [
        "src/protocol_detector.cpp",
    ],
    hdrs = [
        "src/protocol_detector.hpp",
    ],
    strip_include_prefix = "",
)

cc_test(
    name = "protocol_detector_test",
    srcs = ["test/protocol_detector_test.cpp"],
    deps = [
        ":protocol_detector",
        "@googletest//:gtest_main",
    ],
)
