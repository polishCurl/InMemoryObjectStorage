cc_library(
    name = "file_store",
    srcs = [
    ],
    hdrs = [
        "file_store.hpp",
    ],
    strip_include_prefix = "",
)

cc_library(
    name = "memory_store",
    srcs = [
        "src/memory_store/memory_store.cpp",
    ],
    hdrs = [
        "src/memory_store/memory_store.hpp",
    ],
    strip_include_prefix = "",
    deps = [":file_store"],
)

cc_test(
    name = "memory_store_test",
    srcs = ["test/memory_store/memory_store_test.cpp"],
    deps = [
        ":memory_store",
        "@googletest//:gtest_main",
    ],
)
