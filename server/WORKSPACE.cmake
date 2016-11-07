set (BAZEL_ROOT ${WORKSPACE}/bazel-out/local-dbg)
set (BAZEL_BIN ${BAZEL_ROOT}/bin)

set (INC_ROOT ${BAZEL_ROOT}/include)

set (BOOST_INC ${WORKSPACE}/bazel-server/external)

set (GTEST_INC ${WORKSPACE}/bazel-server/external/gtest/include)
set (GTEST_LIB ${BAZEL_BIN}/external/gtest/libmain.a)

set (LMDB_INC ${INC_ROOT}/external/lmdb/_/lmdb)
set (LMDB_LIB ${BAZEL_BIN}/external/lmdb/liblmdb_impl.a)

set (ZLIB_INC ${INC_ROOT}/external/zlib/_/zlib)
set (ZLIB_LIB ${BAZEL_BIN}/external/zlib/libzlib.a)

set (LIBUV_INC ${INC_ROOT}/libuv/_/libuv)
set (LIBUV_LIB ${BAZEL_BIN}/libuv/liblibuv_impl.a)

set (SHA1_INC ${INC_ROOT}/sha1/_/sha1)
set (SHA1_LIB ${BAZEL_BIN}/sha1/libsha1.a)

set (TWEETNACL_INC ${INC_ROOT}/tweetnacl/_/tweetnacl)
set (TWEETNACL_LIB ${BAZEL_BIN}/tweetnacl/libtweetnacl.a)

set (UWEBSOCKETS_INC ${INC_ROOT}/uWebSockets/_/uWebSockets)
set (UWEBSOCKETS_LIB ${BAZEL_BIN}/uWebSockets/libuWebSockets_impl.a ${ZLIB_LIB} ${LIBUV_LIB} ${SHA1_LIB})

set (WEBSOCKETPP_INC ${INC_ROOT}/websocketpp/_/websocketpp)

set (CAPNPROTO_INC ${WORKSPACE}/bazel-server)
set (CAPNPROTO_CAPNP_LIB ${BAZEL_BIN}/capnproto/capnp/libcapnp.a)
set (CAPNPROTO_KJ_LIB ${BAZEL_BIN}/capnproto/kj/libkj.a)

include_directories(${LMDB_INC} ${BOOST_INC} ${GTEST_INC} ${WEBSOCKETPP_INC} ${UWEBSOCKETS_INC} ${LIBUV_INC} ${CAPNPROTO_INC} ${SHA1_INC} ${ZLIB_INC})
