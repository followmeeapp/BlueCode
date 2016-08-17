cc_binary(
  name = "discoveryd",
  srcs = [
    "src/config.h",
    "src/main.cpp",
  ],
  deps = [
    "@boost//:boost",
    "//discovery_engine/engine:engine",
    "//discovery_engine/cpp_lmdb",
    "//discovery_engine/xy_lmdb_graph",
    "//aeron",
    "//schema",
  ],
  copts=[
    "-Iexternal",
    "-std=c++14",
  ],
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
    "//aeron",
    "//schema",
  ],
  size = "small",
  copts = [
    "-std=c++14",
    "-Iexternal/gtest/include",
    "-Iexternal",
  ],
)
