workspace(name = "cpcli")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_jar")

# boost
http_archive(
    name = "com_github_nelhage_rules_boost",
    sha256 = "5ea00abc70cdf396a23fb53201db19ebce2837d28887a08544429d27783309ed",
    strip_prefix = "rules_boost-96e9b631f104b43a53c21c87b01ac538ad6f3b48",
    url = "https://github.com/nelhage/rules_boost/archive/96e9b631f104b43a53c21c87b01ac538ad6f3b48.tar.gz",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

# gson
http_jar(
    name = "gson",
    downloaded_file_name = "gson-2.10.jar",
    sha256 = "0cdd163ce3598a20fc04eee71b140b24f6f2a3b35f0a499dbbdd9852e83fbfaf",
    url = "https://repo1.maven.org/maven2/com/google/code/gson/gson/2.10/gson-2.10.jar",
)

# crow
http_archive(
    name = "crow",
    build_file_content = """
cc_library(
    name = "crow",
    hdrs = glob(["include/*.h", "include/crow/*.h", "include/crow/*.hpp"]),
    strip_include_prefix = "include",
    deps = ["@boost//:optional", "@boost//:algorithm", "@boost//:asio"],
    visibility = ["//visibility:public"],
)
""",
    sha256 = "c299e8ac6c4286139ba14dc9555db9f15902182a2ddcb1e25ca0984f67152877",
    url = "https://github.com/CrowCpp/Crow/releases/download/v1.0%2B5/crow-v1.0+5.tar.gz",
)

# cli11
http_archive(
    name = "cli11",
    build_file_content = """
cc_library(
    name = "cli11",
    hdrs = glob(["include/CLI/*.hpp", "include/CLI/impl/*.hpp"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
""",
    sha256 = "378da73d2d1d9a7b82ad6ed2b5bda3e7bc7093c4034a1d680a2e009eb067e7b2",
    strip_prefix = "CLI11-2.3.1",
    url = "https://github.com/CLIUtils/CLI11/archive/refs/tags/v2.3.1.tar.gz",
)

# nlohmann/json.hpp
http_archive(
    name = "json",
    build_file_content = """
cc_library(
    name = "json",
    hdrs = ["single_include/nlohmann/json.hpp"],
    strip_include_prefix = "single_include",
    visibility = ["//visibility:public"],
)
""",
    sha256 = "d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273",
    strip_prefix = "json-3.11.2",
    url = "https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz",
)

# spdlog/spdlog.h
http_archive(
    name = "spdlog",
    build_file_content = """
cc_library(
    name = "spdlog",
    hdrs = glob(["include/spdlog/**/*.h"]),
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
""",
    sha256 = "ca5cae8d6cac15dae0ec63b21d6ad3530070650f68076f3a4a862ca293a858bb",
    strip_prefix = "spdlog-1.11.0",
    url = "https://github.com/gabime/spdlog/archive/refs/tags/v1.11.0.tar.gz",
)

# inja
http_archive(
    name = "inja",
    build_file_content = """
cc_library(
    name = "inja",
    hdrs = ["single_include/inja/inja.hpp"],
    strip_include_prefix = "single_include/inja",
    visibility = ["//visibility:public"],
)
""",
    sha256 = "a95e95ff39961be429d564689d265a2eb2f269cb180bb0028c842a7484916cb6",
    strip_prefix = "inja-3.3.0",
    url = "https://github.com/pantor/inja/archive/refs/tags/v3.3.0.zip",
)

# gtest
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "gtest",
    commit = "58d77fa8070e8cec2dc1ed015d66b454c8d78850",
    remote = "https://github.com/google/googletest",
    shallow_since = "1656350095 -0400",
)

# Hedron's Compile Commands Extractor for Bazel
# https://github.com/hedronvision/bazel-compile-commands-extractor
http_archive(
    name = "hedron_compile_commands",
    sha256 = "b0a8af42e06108ec62382703daf27f7d8d247fd1b930f249045c70cd9d22f72e",
    strip_prefix = "bazel-compile-commands-extractor-c200ce8b3e0baa04fa5cc3fb222260c9ea06541f",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/c200ce8b3e0baa04fa5cc3fb222260c9ea06541f.tar.gz",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()
