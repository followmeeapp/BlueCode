project (engine)

cmake_minimum_required (VERSION 2.8)

set(CMAKE_MACOSX_RPATH 1)

include(${WORKSPACE}/WORKSPACE.cmake)

include_directories("." "./src")

add_compile_options(-std=c++14)

SET( CPP_FILES
${PROJECT_SOURCE_DIR}/src/engine.cpp
${PROJECT_SOURCE_DIR}/src/session_db.cpp
${PROJECT_SOURCE_DIR}/src/graph_db.cpp
${PROJECT_SOURCE_DIR}/src/generic.cpp
)

SET( H_FILES
${PROJECT_SOURCE_DIR}/src/engine.h
${PROJECT_SOURCE_DIR}/src/session_db.h
${PROJECT_SOURCE_DIR}/src/graph_db.h
${PROJECT_SOURCE_DIR}/src/generic.h
)

SET( SOURCE_FILES ${CPP_FILES} ${H_FILES})

SET(BUILD_FILES
${PROJECT_SOURCE_DIR}/BUILD
${PROJECT_SOURCE_DIR}/PACKAGE.cmake
)

include (${WORKSPACE}/cpp_lmdb/PACKAGE.cmake)
include (${WORKSPACE}/discovery_engine/xy_lmdb_graph/PACKAGE.cmake)
include (${WORKSPACE}/discovery_engine/engine/PACKAGE.cmake)
include (${WORKSPACE}/schema/PACKAGE.cmake)
include (${WORKSPACE}/clock/PACKAGE.cmake)

add_library(engine STATIC ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(engine ${LMDB_LIB} ${CPP_LMDB_LIB} ${SCHEMA_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB})

add_executable(test src/test/test.cpp ${BUILD_FILES})
target_link_libraries(test engine)
target_link_libraries(test ${GTEST_LIB})

add_custom_target (FORMAT ALL
/usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/test/*.cpp
)

