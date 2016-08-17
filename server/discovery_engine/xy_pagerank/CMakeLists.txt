project (xy_pagerank)

cmake_minimum_required (VERSION 2.8)

# The version number.
set (XY_PAGERANK_VERSION_MAJOR 0)
set (XY_PAGERANK_VERSION_MINOR 1)
set (XY_PAGERANK_USE_XYPRIOQUEUE 0)
set (CODEBASE_ROOT ${PROJECT_SOURCE_DIR}/.. CACHE PATH "The root path of the entire code base")

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
"src/xy_pagerank_config.h.in"
"${PROJECT_SOURCE_DIR}/src/xy_pagerank_config.h"
)

SET( CPP_FILES
${PROJECT_SOURCE_DIR}/src/xy_pagerank_runner.cpp
)

SET( H_FILES
${PROJECT_SOURCE_DIR}/src/xy_pagerank.h
${PROJECT_SOURCE_DIR}/src/xy_graph_writer.h
${PROJECT_SOURCE_DIR}/src/xy_pagerank_config.h
)
SET( SOURCE_FILES ${CPP_FILES} ${H_FILES} )

include (${CODEBASE_ROOT}/exports/cpp_lmdb_c-exports.cmake)

include_directories("." ${CODEBASE_ROOT}/xy_lmdb_graph/src ${CODEBASE_ROOT}/cpp_lmdb_c/include)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS program_options)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_compile_options(-stdlib=libc++)
add_compile_options(-std=c++14)

add_executable(xy_pagerank_old src/main.cpp src/xy_pagerank_old.h src/xy_graph_writer.h src/xy_priority_queue.h src/xy_pagerank_config.h)
target_link_libraries(xy_pagerank_old ${Boost_LIBRARIES})

add_executable(xy_pagerank_c ${SOURCE_FILES})
target_link_libraries(xy_pagerank_c ${Boost_LIBRARIES} cpp_lmdb_c)

add_executable(XYPriorityQueueTest src/test/xy_priority_queue_test.cpp src/xy_priority_queue.h)
target_link_libraries(XYPriorityQueueTest ${GTEST_BOTH_LIBRARIES})

GTEST_ADD_TESTS(XYPriorityQueueTest "" AUTO)

add_custom_target (FORMAT ALL
/usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/src/main.cpp ${PROJECT_SOURCE_DIR}/src/xy_pagerank_old.h ${PROJECT_SOURCE_DIR}/src/test/*.cpp
)