cc_binary(
  name = "discoveryd",
  srcs = [
    "src/config.h",
    "src/discoveryd.h",
    "src/discoveryd.cpp",
    "src/main.cpp",
  ],
  deps = [
    "@boost//:boost",
    "//discovery_engine/engine:engine",
    "//cpp_lmdb",
    "//discovery_engine/xy_lmdb_graph",
    "//aeron",
    "//schema",
    "//clock",
  ],
  copts=[
    "-Iexternal",
    "-std=c++14",
  ],
)

cc_inc_library(
  name = "discovery_engine",
  hdrs = [
    "src/discoveryd.h",
  ],
  prefix = "src",
  deps = [ "//discovery_engine/engine" ],
  visibility = ["//visibility:public"],
)

cc_test(
  name="test",
  srcs=[
    "src/config.h",
    "src/test/test.cpp",
  ],
  deps=[
    "@gtest//:main",
    "@boost//:boost",
    "//discovery_engine",
    "//aeron",
    "//schema",
    "//clock",
    "//test_process_runner",
  ],
  size = "small",
  copts = [
    "-std=c++14",
    "-Iexternal/gtest/include",
    "-Iexternal",
  ],
)
