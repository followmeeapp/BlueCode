project (xy_lmdb_graph)

cmake_minimum_required (VERSION 2.8)

set(CMAKE_MACOSX_RPATH 1)

include(${WORKSPACE}/WORKSPACE.cmake)

SET( CPP_FILES
${PROJECT_SOURCE_DIR}/src/load_graph.cpp
)

SET( H_FILES
${PROJECT_SOURCE_DIR}/src/xy_lmdb_graph.h
${PROJECT_SOURCE_DIR}/src/xy_graph_edge_list.h
)
SET( SOURCE_FILES ${CPP_FILES} ${H_FILES} )

SET(BUILD_FILES
${PROJECT_SOURCE_DIR}/BUILD
${PROJECT_SOURCE_DIR}/PACKAGE.cmake
)
include (${WORKSPACE}/discovery_engine/cpp_lmdb/PACKAGE.cmake)
include (${WORKSPACE}/discovery_engine/xy_lmdb_graph/PACKAGE.cmake)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS program_options)

add_compile_options(-std=c++14)

add_executable(load_graph ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(load_graph ${Boost_LIBRARIES} ${LMDB_LIB} ${CPP_LMDB_LIB})

add_executable(test src/test/test.cpp ${H_FILES} ${BUILD_FILES})
target_link_libraries(test ${Boost_LIBRARIES} ${GTEST_LIB} ${LMDB_LIB} ${CPP_LMDB_LIB})

add_custom_target (FORMAT ALL
/usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/test/*.cpp
)