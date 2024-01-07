cc_library(
    name = "object_storage",
    srcs = [
        "src/object_storage.cpp",
    ],
    hdrs = [
        "src/object_storage.hpp",
    ],
    deps = [
        "//server:server_interface",
        "//user/database:user_database",
        "@boost//:asio",
        "@boost//:log",
    ],
)

cc_test(
    name = "object_storage_test",
    srcs = ["test/object_storage_test.cpp"],
    deps = [
        ":object_storage",
        "@googletest//:gtest_main",
    ],
)
