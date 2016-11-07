cc_binary(
name = "dashboard_server",
srcs = [
"src/config.h",
"src/generic.h",
"src/main.cpp",
],
deps = [
"//dashboard_server/engine",
"@boost//:boost",
"//traildb-cpp",
"//aeron",
"//archive",
"//schema",
"//cpp_lmdb",
"//uWebSockets",
"//rapidjson",
],
copts=[
"-Iexternal",
"-std=c++14",
"-DBAZEL=1",
"-DNO_OPENSSL",
],
)

cc_test(
name="test",
srcs=[
"src/config.h",
"src/generic.h",
"src/test/test.cpp",
],
deps=[
"@gtest//:main",
"@boost//:boost",
"//cpp_lmdb",
"//traildb-cpp",
"//archive",
"//schema",
"//websocketpp",
"//rapidjson",
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
