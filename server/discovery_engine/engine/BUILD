cc_inc_library(
  name = "engine",
  hdrs=[
    "src/engine.h",
    "src/generic.h",
    "src/session_db.h",
    "src/graph_db.h",
  ],
  deps=[":engine_impl"],
  prefix = "src",
  visibility = ["//visibility:public"],
)

cc_library(
  name="engine_impl",
  hdrs=[
    "src/engine.h",
    "src/generic.h",
    "src/session_db.h",
    "src/graph_db.h",
  ],
  srcs=[
    "src/engine.cpp",
    "src/generic.cpp",
    "src/session_db.cpp",
    "src/graph_db.cpp",
  ],
  deps=[
    "//cpp_lmdb",
    "//discovery_engine/xy_lmdb_graph",
    "//schema",
    "//clock",
    "@boost//:boost"
  ],
  linkstatic=1,
  copts=[
    "-Iexternal",
    "-std=c++14",
  ],
  visibility = ["//visibility:private"],
)

cc_test(
  name="test",
  srcs=[
    "src/test/test.cpp",
  ],
  deps=[
    ":engine",
    "@gtest//:main",
  ],
  size = "small",
  copts = [
    "-std=c++14",
    "-Iexternal/gtest/include",
    "-Iexternal"
  ],
)
