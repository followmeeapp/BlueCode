//
//  event_logger.h
//  event_logger
//
//  Created by Erik van der Tier on 05/09/16.
//
//

#ifndef EVENT_LOGGER__H_
#define EVENT_LOGGER__H_

#include "cpp_lmdb/lmdb_env.h"

namespace EVENT_LOGGER {
  
  auto TraceKeyCmp(const MDB_val *val1, const MDB_val *val2) -> int;
  
  struct trace_key {
    int64_t time;
    int16_t request_type;
  };

  auto HistogramKeyCmp(const MDB_val *val1, const MDB_val *val2) -> int;

  struct histogram_key {
    uint8_t interval;
    uint64_t position;
  };
} // namespace EVENT_LOGGER

#endif // EVENT_LOGGER__H_
