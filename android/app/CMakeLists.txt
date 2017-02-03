cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_MACOSX_RPATH 1)

set (KJ_SOURCE_DIR /Users/erik/Suiron/Code/xygroup/capnproto/kj)

include_directories("/Users/erik/Suiron/Code/xygroup/")

set(kj_sources_lite
  ${KJ_SOURCE_DIR}/array.c++
  ${KJ_SOURCE_DIR}/common.c++
  ${KJ_SOURCE_DIR}/debug.c++
  ${KJ_SOURCE_DIR}/exception.c++
  ${KJ_SOURCE_DIR}/io.c++
  ${KJ_SOURCE_DIR}/memory.c++
  ${KJ_SOURCE_DIR}/mutex.c++
  ${KJ_SOURCE_DIR}/string.c++
  ${KJ_SOURCE_DIR}/thread.c++
  ${KJ_SOURCE_DIR}/main.c++
  ${KJ_SOURCE_DIR}/arena.c++
)

set(kj_sources ${kj_sources_lite})

add_library(kj ${kj_sources})
add_library(CapnProto::kj ALIAS kj)
  #target_link_libraries(kj PUBLIC pthread)

set (CAPNP_SOURCE_DIR /Users/erik/Suiron/Code/xygroup/capnproto/capnp)

set(capnp_sources_lite
  ${CAPNP_SOURCE_DIR}/c++.capnp.c++
  ${CAPNP_SOURCE_DIR}/blob.c++
  ${CAPNP_SOURCE_DIR}/arena.c++
  ${CAPNP_SOURCE_DIR}/layout.c++
  ${CAPNP_SOURCE_DIR}/list.c++
  ${CAPNP_SOURCE_DIR}/any.c++
  ${CAPNP_SOURCE_DIR}/message.c++
  ${CAPNP_SOURCE_DIR}/schema.capnp.c++
  ${CAPNP_SOURCE_DIR}/serialize.c++
  ${CAPNP_SOURCE_DIR}/serialize-packed.c++
)

add_library(capnp ${capnp_sources_lite})
add_library(CapnProto::capnp ALIAS capnp)
target_link_libraries(capnp kj)

add_library(capnp_echo SHARED src/main/cpp/CapnpEcho.cpp)
target_link_libraries(capnp_echo capnp kj)

# The version number.
set (LIBLMDB_VERSION_MAJOR 0)
set (LIBLMDB_VERSION_MINOR 1)
set (CODEBASE_ROOT ${PROJECT_ROOT_DIR}/.. CACHE PATH "The root path of the entire code base")
set (LMDB_SOURCE_DIR /Users/erik/Suiron/Code/xygroup/liblmdb)

include_directories(“${LMDB_SOURCE_DIR}/src”)

SET( LMDB_C_FILES
${LMDB_SOURCE_DIR}/src/mdb.c
${LMDB_SOURCE_DIR}/src/midl.c
src/main/cpp/LMDBJNI.cpp
)

SET( LMDB_H_FILES
${LMDB_SOURCE_DIR}/src/lmdb.h
${LMDB_SOURCE_DIR}/src/midl.h
)

SET( LMDB_SOURCE_FILES ${LMDB_C_FILES} ${LMDB_H_FILES} )

add_library(lmdb SHARED ${LMDB_SOURCE_FILES})
target_link_libraries(lmdb capnp kj
                      android
                      log)