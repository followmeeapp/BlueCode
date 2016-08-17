cc_inc_library(
  name = "lmdb",
  hdrs = ["libraries/liblmdb/lmdb.h"],
  deps = [":lmdb_impl"],
  prefix = "libraries/liblmdb",
  visibility = ["//visibility:public"],
)

cc_library(
  name = "lmdb_impl",
  srcs = [
    "libraries/liblmdb/lmdb.h",
    "libraries/liblmdb/mdb.c",
    "libraries/liblmdb/midl.c",
    "libraries/liblmdb/midl.h"],
  hdrs = ["libraries/liblmdb/lmdb.h"],
  linkstatic = 1,
  copts = [
    "-W",
    "-Wall",
    "-Wno-unused-parameter",
    "-Wbad-function-cast",
    "-Wuninitialized",
  ],
  visibility = ["//visibility:private"],
)
