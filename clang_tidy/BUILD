filegroup(
    name = "clang_tidy_config_default",
    srcs = [
        ".clang-tidy",
        # '//example:clang_tidy_config', # add package specific configs if needed
    ],
)

label_flag(
    name = "clang_tidy_config",
    build_setting_default = ":clang_tidy_config_default",
    visibility = ["//visibility:public"],
)

filegroup(
    name = "clang_tidy_executable_default",
    srcs = [],  # empty list: system clang-tidy
)

label_flag(
    name = "clang_tidy_executable",
    build_setting_default = ":clang_tidy_executable_default",
    visibility = ["//visibility:public"],
)

filegroup(
    name = "clang_tidy_additional_deps_default",
    srcs = [],
)

label_flag(
    name = "clang_tidy_additional_deps",
    build_setting_default = ":clang_tidy_additional_deps_default",
    visibility = ["//visibility:public"],
)

sh_binary(
    name = "clang_tidy",
    srcs = ["run_clang_tidy.sh"],
    data = [":clang_tidy_config"],
    visibility = ["//visibility:public"],
)
