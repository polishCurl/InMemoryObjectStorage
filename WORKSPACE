load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Google Test (gtest)
http_archive(
    name = "googletest",
    strip_prefix = "googletest-5126f7166109666a9c0534021fb1a3038659494c",
    urls = ["https://github.com/google/googletest/archive/5126f7166109666a9c0534021fb1a3038659494c.zip"],
)

# Google benchmark
http_archive(
    name = "googlebench",
    strip_prefix = "benchmark-920fa14898d055d61b399160981271a45f49832a",
    urls = ["https://github.com/google/benchmark/archive/920fa14898d055d61b399160981271a45f49832a.zip"],
)

# Boost
http_archive(
    name = "com_github_nelhage_rules_boost",
    strip_prefix = "rules_boost-96e9b631f104b43a53c21c87b01ac538ad6f3b48",
    url = "https://github.com/nelhage/rules_boost/archive/96e9b631f104b43a53c21c87b01ac538ad6f3b48.tar.gz",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()
