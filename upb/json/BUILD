# TODO: describe this package.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("//bazel:proto_library.bzl", "proto_library")
load(
    "//bazel:upb_proto_library.bzl",
    "upb_c_proto_library",
    "upb_proto_reflection_library",
)
load("//upb/bazel:build_defs.bzl", "UPB_DEFAULT_COPTS")

package(default_applicable_licenses = ["//:license"])

cc_library(
    name = "json",
    srcs = [
        "decode.c",
        "encode.c",
    ],
    hdrs = [
        "decode.h",
        "encode.h",
    ],
    copts = UPB_DEFAULT_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//upb/base",
        "//upb/lex",
        "//upb/mem",
        "//upb/message",
        "//upb/mini_table",
        "//upb/port",
        "//upb/reflection",
        "//upb/wire",
    ],
)

cc_test(
    name = "decode_test",
    srcs = ["decode_test.cc"],
    deps = [
        ":json",
        ":test_upb_proto",
        ":test_upb_proto_reflection",
        "//:struct_upb_proto",
        "//upb/base",
        "//upb/mem",
        "//upb/reflection",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "encode_test",
    srcs = ["encode_test.cc"],
    deps = [
        ":json",
        ":test_upb_proto",
        ":test_upb_proto_reflection",
        "//:struct_upb_proto",
        "//upb/base",
        "//upb/mem",
        "//upb/reflection",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

proto_library(
    name = "test_proto",
    testonly = 1,
    srcs = ["test.proto"],
    deps = ["//:struct_proto"],
)

upb_c_proto_library(
    name = "test_upb_proto",
    testonly = 1,
    deps = [":test_proto"],
)

upb_proto_reflection_library(
    name = "test_upb_proto_reflection",
    testonly = 1,
    deps = [":test_proto"],
)

filegroup(
    name = "test_srcs",
    srcs = glob(
        [
            "**/*test.cc",
        ],
        exclude = ["fuzz_test.cc"],
    ),
    visibility = ["//upb:__pkg__"],
)

filegroup(
    name = "test_protos",
    srcs = glob(
        [
            "**/*test.proto",
        ],
    ),
    visibility = ["//upb:__pkg__"],
)
