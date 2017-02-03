project (wsserver)

cmake_minimum_required (VERSION 2.8)

include(${WORKSPACE}/WORKSPACE.cmake)

include_directories("." "./src")

add_compile_options(-std=c++14)
add_compile_options(-DBAZEL=1)
add_compile_options(-DNO_OPENSSL)

SET( CPP_FILES
${PROJECT_SOURCE_DIR}/src/main.cpp
)
SET( H_FILES
${PROJECT_SOURCE_DIR}/src/config.h
${PROJECT_SOURCE_DIR}/src/generic.h
)

SET( SOURCE_FILES ${CPP_FILES} ${H_FILES})

SET(BUILD_FILES
${PROJECT_SOURCE_DIR}/BUILD
)

include (${WORKSPACE}/schema/PACKAGE.cmake)

add_executable(wsserver ${SOURCE_FILES} ${BUILD_FILES})
target_link_libraries(wsserver ${SCHEMA_LIB} ${CAPNPROTO_CAPNP_LIB} ${CAPNPROTO_KJ_LIB} ${UWEBSOCKETS_LIB} )
