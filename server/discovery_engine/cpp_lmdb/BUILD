cc_inc_library(
  name = "cpp_lmdb",
  hdrs = [
    "src/lmdb_env.h",
    "src/lmdb_database.h",
    "src/lmdb_cursor.h",
    "src/lmdb_dbval.h",
  ],
  deps = [
    ":cpp_lmdb_impl"
  ],
  prefix = "src",
  visibility = ["//visibility:public"]
)

cc_library(
  name = "cpp_lmdb_impl",
  srcs = [
    "src/lmdb_env.cpp",
    "src/lmdb_env.h",
    "src/lmdb_database.h",
    "src/lmdb_dbval.h",
    "src/lmdb_cursor.h",
  ],
  hdrs = [
    "src/lmdb_env.h",
    "src/lmdb_database.h",
    "src/lmdb_cursor.h",
    "src/lmdb_dbval.h",
  ],
  deps = [
    "@lmdb//:lmdb",
    "@boost//:boost"
  ],
  linkstatic = 1,
  copts = [
    "-std=c++14",
    "-Iexternal/libraries",
    "-Iexternal"
  ],
  visibility = ["//visibility:private"],
)

cc_test(name = "test", srcs = ["test/test.cpp"],
  deps = [
    ":cpp_lmdb",
    "@lmdb//:lmdb",
    "@gtest//:main"
  ],
  linkstatic = 1,
  size = "small",
  copts = [
    "-std=c++14",
    "-Iexternal",
    "-Iexternal/libraries",
    "-Iexternal/gtest/include"
  ],
)
