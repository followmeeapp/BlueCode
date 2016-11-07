project (event_logger)

cmake_minimum_required (VERSION 2.8)

set(CMAKE_MACOSX_RPATH 1)

include(${WORKSPACE}/WORKSPACE.cmake)

include_directories("." "./src")

add_compile_options(-std=c++14)
add_compile_options(-DBAZEL=1)

SET( CPP_FILES
${PROJECT_SOURCE_DIR}/src/main.cpp
)
SET( H_FILES
${PROJECT_SOURCE_DIR}/src/config.h
)

SET( SOURCE_FILES ${CPP_FILES} ${H_FILES})

SET(BUILD_FILES
${PROJECT_SOURCE_DIR}/BUILD
${PROJECT_SOURCE_DIR}/PACKAGE.cmake
)

include (${WORKSPACE}/traildb-cpp/PACKAGE.cmake)
include (${WORKSPACE}/aeron/PACKAGE.cmake)
include (${WORKSPACE}/schema/PACKAGE.cmake)
include (${WORKSPACE}/event_logger/engine/PACKAGE.cmake)
include (${WORKSPACE}/event_logger/PACKAGE.cmake)
include (${WORKSPACE}/cpp_lmdb/PACKAGE.cmake)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system)

add_library(event_logger_h ${PROJECT_SOURCE_DIR}/src/event_logger.h ${PROJECT_SOURCE_DIR}/src/event_logger.cpp)
target_link_libraries(event_logger_h ${CPP_LMDB_LIB})

add_executable(event_logger ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(event_logger ${EVENT_LOGGER_LIB} ${ENGINE_LIB} ${CPP_LMDB_LIB} ${LMDB_LIB} ${TRAILDB_LIB} ${SCHEMA_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB} ${AERON_LIB})

add_executable(test ${PROJECT_SOURCE_DIR}/src/test/test.cpp ${BUILD_FILES})
target_link_libraries(test ${Boost_LIBRARIES} ${SCHEMA_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB} ${AERON_LIB})
target_link_libraries(test ${GTEST_LIB})

add_custom_target (FORMAT ALL
/usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/test/test.cpp
)
