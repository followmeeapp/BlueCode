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

cc_binary(
  name = "mdb_copy",
  srcs = [
    "libraries/liblmdb/mdb_copy.c",
    "libraries/liblmdb/lmdb.h",
    "libraries/liblmdb/mdb.c",
    "libraries/liblmdb/midl.c",
    "libraries/liblmdb/midl.h",
  ],
  copts = [
    "-W",
    "-Wall",
    "-Wno-unused-parameter",
    "-Wbad-function-cast",
    "-Wuninitialized",
  ],
  deps = [],
)

cc_binary(
  name = "mdb_dump",
  srcs = [
    "libraries/liblmdb/mdb_dump.c",
    "libraries/liblmdb/lmdb.h",
    "libraries/liblmdb/mdb.c",
    "libraries/liblmdb/midl.c",
    "libraries/liblmdb/midl.h",
  ],
  copts = [
    "-W",
    "-Wall",
    "-Wno-unused-parameter",
    "-Wbad-function-cast",
    "-Wuninitialized",
  ],
  deps = [],
)

cc_binary(
  name = "mdb_load",
  srcs = [
    "libraries/liblmdb/mdb_load.c",
    "libraries/liblmdb/lmdb.h",
    "libraries/liblmdb/mdb.c",
    "libraries/liblmdb/midl.c",
    "libraries/liblmdb/midl.h",
  ],
  copts = [
    "-W",
    "-Wall",
    "-Wno-unused-parameter",
    "-Wbad-function-cast",
    "-Wuninitialized",
  ],
  deps = [],
)

cc_binary(
  name = "mdb_stat",
  srcs = [
    "libraries/liblmdb/mdb_stat.c",
    "libraries/liblmdb/lmdb.h",
    "libraries/liblmdb/mdb.c",
    "libraries/liblmdb/midl.c",
    "libraries/liblmdb/midl.h",
  ],
  copts = [
    "-W",
    "-Wall",
    "-Wno-unused-parameter",
    "-Wbad-function-cast",
    "-Wuninitialized",
  ],
  deps = [],
)
