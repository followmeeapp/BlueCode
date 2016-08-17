project (cpp_lmdb)

cmake_minimum_required (VERSION 2.8)

set(CMAKE_MACOSX_RPATH 1)

include(${WORKSPACE}/WORKSPACE.cmake)

include_directories(“.” “./src”)

add_compile_options(-std=c++14)

SET( CPP_FILES
	${PROJECT_SOURCE_DIR}/src/lmdb_env.cpp
)

SET( H_FILES
  ${PROJECT_SOURCE_DIR}/src/lmdb_dbval.h
	${PROJECT_SOURCE_DIR}/src/lmdb_env.h
	${PROJECT_SOURCE_DIR}/src/lmdb_database.h
	${PROJECT_SOURCE_DIR}/src/lmdb_cursor.h
)
SET( SOURCE_FILES ${CPP_FILES} ${H_FILES})

SET(BUILD_FILES
${PROJECT_SOURCE_DIR}/BUILD
${PROJECT_SOURCE_DIR}/PACKAGE.cmake
)

add_library(cpp_lmdb STATIC ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(cpp_lmdb ${LMDB_LIB})

add_executable(test test/test.cpp ${BUILD_FILES})
target_link_libraries(test cpp_lmdb)
target_link_libraries(test ${GTEST_LIB})

add_custom_target (FORMAT ALL
/usr/local/bin/clang-format -style=Google -i ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/test/*.cpp
)

