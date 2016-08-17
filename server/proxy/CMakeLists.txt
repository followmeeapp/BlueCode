project (proxy)

cmake_minimum_required (VERSION 3.1.0)

set(CMAKE_MACOSX_RPATH 1)

include(${WORKSPACE}/WORKSPACE.cmake)

include_directories(".")

add_compile_options(-stdlib=libc++)
add_compile_options(-std=c++14)
add_compile_options(-pthread)

# required for libuv
# add_compile_options(-D_LARGEFILE_SOURCE)
# add_compile_options(-D_FILE_OFFSET_BITS=64)

SET( CPP_FILES
    ${PROJECT_SOURCE_DIR}/blinky.cpp
)
SET( H_FILES
)

SET( SOURCE_FILES ${CPP_FILES} ${H_FILES})

SET( BUILD_FILES
  ${PROJECT_SOURCE_DIR}/BUILD
)

include (${WORKSPACE}/qpc/PACKAGE.cmake)
include (${WORKSPACE}/rpc/PACKAGE.cmake)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(proxy ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(proxy ${QPC_LIB} ${Boost_LIBRARIES} ${RPC_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB})
target_link_libraries(proxy ${CMAKE_THREAD_LIBS_INIT})

add_custom_target (FORMAT ALL
    /usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES}
)
