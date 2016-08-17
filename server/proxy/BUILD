cc_binary(
    name = "proxy",
    srcs = [
        "blinky.cpp",
    ],
    copts = [
        "-Iexternal", # needed for Boost for some reason?
        "-std=c++14",
        "-stdlib=libc++",
        "-DQP_API_VERSION=9999",
    ],
    deps = [
        "@boost//:boost",
        "//qpc",
        "//schema",
    ],
)
