cc_library(
    name = "user_database",
    srcs = [
        "src/user_database.cpp",
    ],
    hdrs = [
        "src/user_database.hpp",
    ],
    deps = [
        "//user:user_database_interface",
        "@boost//:asio",
    ],
)

cc_test(
    name = "user_database_test",
    srcs = ["test/user_database_test.cpp"],
    deps = [
        ":user_database",
        "@googletest//:gtest_main",
    ],
)
