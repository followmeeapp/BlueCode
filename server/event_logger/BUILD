cc_inc_library(
name = "event_logger_h",
hdrs = ["src/event_logger.h"],
deps = [":event_logger_h_impl"],
prefix = "src",
visibility = ["//event_logger:__subpackages__"],
)

cc_library(
name = "event_logger_h_impl",
srcs = [
"src/event_logger.h",
"src/event_logger.cpp",
],
deps = [
"//cpp_lmdb",
],
linkstatic=1,
copts=[
"-DBAZEL=1",
"-Iexternal",
"-std=c++14",
],
visibility = ["//visibility:private"],
)

cc_binary(
name = "event_logger",
srcs = [
"src/config.h",
"src/main.cpp",
],
deps = [
"//event_logger/engine",
"@boost//:boost",
"//schema",
"//aeron",
"//cpp_lmdb",
],
copts=[
"-Iexternal",
"-std=c++14",
"-DBAZEL=1",
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
"//websocketpp",
],
size = "small",
copts = [
"-std=c++14",
"-Iexternal/gtest/include",
"-Iexternal",
"-DBAZEL=1",
"-DNO_OPENSSL",
],
)
