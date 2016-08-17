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
        "//uWebSockets",
        "@lmdb//:lmdb",
        "//capnproto/capnp",
        "//schema",
        "//aeron",
        "//libuv",
    ],
)

cc_binary(
    name = "nop",
    srcs = [
        "nop.cpp",
    ],
    deps = [
        "//uWebSockets",
        "//capnproto/capnp",
        "//rpc",
    ],
)
