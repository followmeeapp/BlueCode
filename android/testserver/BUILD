cc_binary(
name = "wsserver",
srcs = [
"src/config.h",
"src/generic.h",
"src/main.cpp",
],
deps = [
"@boost//:boost",
"//schema",
"//uWebSockets",
],
copts=[
"-Iexternal",
"-Iexternal/boost",
"-std=c++14",
"-DBAZEL=1",
"-DNO_OPENSSL",
],
)
