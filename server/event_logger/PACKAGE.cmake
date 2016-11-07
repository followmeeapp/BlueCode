set (EVENT_LOGGER_INC ${INC_ROOT}/event_logger/_/event_logger_h)
set (EVENT_LOGGER_LIB ${BAZEL_BIN}/event_logger/libevent_logger_h_impl.a)

include_directories(${EVENT_LOGGER_INC})
