cc_binary(
    name = "main",
    srcs = [
        "main.cpp",
    ],
    copts = [
        "-DBAZEL",
        "-DNO_OPENSSL",
    ],
    deps = [
        ":server",
        "//uWebSockets",
        "//aeron",
        "//libuv",
        "//tweetnacl",
    ],
)

cc_library(
    name = "server",
    srcs =  [
        "server.cpp",
        "stacktrace.cpp",
    ],
    hdrs = [
        "server.h",
        "stacktrace.h",
    ],
    deps = [
        "@lmdb//:lmdb",
        "//capnproto/capnp",
        "//schema",
    ],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test_main",
    srcs = [
        "test_main.cpp",
    ],
    copts = [
        "-stdlib=libc++",
        "-std=c++14",
        "-Iexternal/gtest/include",
    ],
    deps = [
        ":server",
        "@gtest//:main",
    ],
    size = "small",
)
