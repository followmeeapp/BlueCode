cc_inc_library(
  name = "engine",
  hdrs=[
    "src/engine.h",
    "src/generic.h",
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
    "src/event_policy.h",
  ],
  srcs=[
    "src/engine.cpp",
    "src/generic.cpp",
    "src/event_policy.cpp",
  ],
  deps=[
    "//event_logger:event_logger_h",
    "//cpp_lmdb",
    "//traildb-cpp",
    "//archive",
    "//schema",
    "//hdr_histogram:hdr_histogram",
    "@boost//:boost"
  ],
  linkstatic=1,
  copts=[
    "-DBAZEL=1",
    "-Iexternal",
    "-std=c++14",
  ],
  visibility = ["//visibility:private"],
)

cc_test(
  name="test",
  srcs=[
    "src/generic.h",
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
