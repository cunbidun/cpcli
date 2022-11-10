workspace(name = "competitive_programming_cli")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# boost
http_archive(
    name = "com_github_nelhage_rules_boost",
    sha256 = "5ea00abc70cdf396a23fb53201db19ebce2837d28887a08544429d27783309ed",
    url = "https://github.com/nelhage/rules_boost/archive/96e9b631f104b43a53c21c87b01ac538ad6f3b48.tar.gz",
    strip_prefix = "rules_boost-96e9b631f104b43a53c21c87b01ac538ad6f3b48",
)
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

# crow 
http_archive(
    name = "crow",
    sha256 = "c299e8ac6c4286139ba14dc9555db9f15902182a2ddcb1e25ca0984f67152877",
    url = "https://github.com/CrowCpp/Crow/releases/download/v1.0%2B5/crow-v1.0+5.tar.gz",
    build_file_content = """
cc_library(
    name = "crow",
    hdrs = glob(["include/*.h", "include/crow/*.h", "include/crow/*.hpp"]),
    strip_include_prefix = "include",
    deps = ["@boost//:optional", "@boost//:algorithm", "@boost//:asio"],
    visibility = ["//visibility:public"],
)
""",
)

# cxxopts 
http_archive(
    name = "cxxopts",
    sha256 = "36f41fa2a46b3c1466613b63f3fa73dc24d912bc90d667147f1e43215a8c6d00",
    url = "https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.0.0.tar.gz",
    strip_prefix = "cxxopts-3.0.0",
    build_file_content = """
cc_library(
    name = "cxxopts",
    hdrs = ["include/cxxopts.hpp"],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
""",
)

# nlohmann/json.hpp
http_archive(
    name = "json",
    url = "https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz",
    sha256 = "d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273",
    strip_prefix = "json-3.11.2",
    build_file_content = """
cc_library(
    name = "json",
    hdrs = ["single_include/nlohmann/json.hpp"],
    strip_include_prefix = "single_include",
    visibility = ["//visibility:public"],
)
""",
)

# spdlog/spdlog.h
http_archive(
    name = "spdlog",
    sha256 = "ca5cae8d6cac15dae0ec63b21d6ad3530070650f68076f3a4a862ca293a858bb",
    url = "https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz",
    strip_prefix = "spdlog-1.11.0",
    build_file_content = """
cc_library(
    name = "spdlog",
    hdrs = glob(["include/spdlog/**/*.h"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
""",
)