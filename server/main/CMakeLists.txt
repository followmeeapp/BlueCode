project (main)

cmake_minimum_required (VERSION 3.1.0)

set(CMAKE_MACOSX_RPATH 1)

include(${WORKSPACE}/WORKSPACE.cmake)

include_directories(".")

add_compile_options(-stdlib=libc++)
add_compile_options(-std=c++14)
add_compile_options(-pthread)

# required for libuv
add_compile_options(-D_LARGEFILE_SOURCE)
add_compile_options(-D_FILE_OFFSET_BITS=64)

#required for uWebSockets
add_compile_options(-DBAZEL)
add_compile_options(-DNO_OPENSSL)

SET( CPP_FILES
)
SET( H_FILES
)

SET( SOURCE_FILES ${CPP_FILES} ${H_FILES})

SET( BUILD_FILES
  ${PROJECT_SOURCE_DIR}/BUILD
  ${PROJECT_SOURCE_DIR}/PACKAGE.cmake
)

include (${WORKSPACE}/aeron/PACKAGE.cmake)
include (${WORKSPACE}/schema/PACKAGE.cmake)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(main ${PROJECT_SOURCE_DIR}/main.cpp ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(main ${ENGINE_LIB} ${UWEBSOCKETS_LIB} ${CPP_LMDB_LIB} ${LMDB_LIB} ${SCHEMA_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB} ${AERON_LIB} ${LIBUV_LIB} ${SHA1_LIB} ${ZLIB_LIB})
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})

add_executable(nop ${PROJECT_SOURCE_DIR}/nop.cpp ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(nop ${ENGINE_LIB} ${UWEBSOCKETS_LIB} ${CPP_LMDB_LIB} ${LMDB_LIB} ${SCHEMA_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB} ${LIBUV_LIB} ${SHA1_LIB} ${ZLIB_LIB})
target_link_libraries(nop ${CMAKE_THREAD_LIBS_INIT})

add_custom_target (FORMAT ALL
    /usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES}
)
