cc_inc_library(
  name="xy_lmdb_graph",
  hdrs=[
    "src/xy_lmdb_graph.h"
  ],
  deps=[
    "//cpp_lmdb",
    "@boost//:boost"
  ],
  prefix="src",
  visibility = ["//visibility:public"],
)

cc_test(
  name="test",
  srcs=[
  "src/test/test.cpp",
  ],
  deps=[
  ":xy_lmdb_graph",
  "//cpp_lmdb",
  "@gtest//:main",
  "@boost//:boost"
  ],
  copts = [
  "-std=c++14",
  "-Iexternal/gtest/include",
  "-Iexternal"],
)

cc_binary(
  name="load_graph",
  srcs=[
    "src/load_graph.cpp",
  ],
  deps=[
    ":xy_lmdb_graph",
    "//cpp_lmdb",
    "@boost//:boost"
  ],
  linkstatic=1,
  copts = [
    "-std=c++14",
    "-Iexternal"],
)



