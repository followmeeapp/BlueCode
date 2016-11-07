cc_binary(
name = "request_histogrammer",
srcs = [
"src/config.h",
"src/main.cpp",
],
deps = [
"//event_logger/request_histogrammer/engine",
"//event_logger:event_logger_h",
"@boost//:boost",
"//schema",
"//cpp_lmdb",
"//traildb-cpp",
"//archive",
"//aeron",
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
"//schema",
"//cpp_lmdb",
"//traildb-cpp",
"//aeron",
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
