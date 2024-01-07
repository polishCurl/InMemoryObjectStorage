cc_library(
    name = "memory_fs",
    srcs = [
        "src/memory_fs.cpp",
    ],
    hdrs = [
        "src/memory_fs.hpp",
    ],
    visibility = ["//server/object_storage:__subpackages__"],
    deps = ["//filesystem:filesystem_interface"],
)

cc_test(
    name = "memory_fs_test",
    srcs = ["test/memory_fs_test.cpp"],
    deps = [
        ":memory_fs",
        "@googletest//:gtest_main",
    ],
)
