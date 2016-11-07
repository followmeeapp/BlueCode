set (ENGINE_INC ${INC_ROOT}/dashboard_server/engine/_/engine)
set (ENGINE_LIB ${BAZEL_BIN}/dashboard_server/engine/libengine_impl.a)

include_directories(${ENGINE_INC})
