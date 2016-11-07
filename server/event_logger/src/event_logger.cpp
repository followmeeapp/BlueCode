//
//  event_logger.cpp
//  event_logger
//
//  Created by Erik van der Tier on 05/09/16.
//
//

#include "event_logger.h"

namespace EVENT_LOGGER {
// higher time is more recent (LT)
// to make sure values with the same timestamp but different id's are sorted
//    lower to higher id values, this should not occur but better be safe than
//    sorry. Has no performance impact at all as this code will not be reached
//    unless somethings wrong
auto TraceKeyCmp(const MDB_val *val1, const MDB_val *val2) -> int {
  auto v1 = static_cast<trace_key *>(val1->mv_data);
  auto v2 = static_cast<trace_key *>(val2->mv_data);
  if (v1->time < v2->time)
    return -1;
  else if (v1->time > v2->time)
    return 1;
  else {
    if (v1->request_type < v2->request_type)
      return -1;
    else if (v1->request_type > v2->request_type)
      return 1;
  }
  return 0;
}
  
  auto HistogramKeyCmp(const MDB_val *val1, const MDB_val *val2) -> int {
    auto v1 = static_cast<histogram_key *>(val1->mv_data);
    auto v2 = static_cast<histogram_key *>(val2->mv_data);
    if (v1->interval < v2->interval) {
      return -1;
    } else if (v1->interval > v2->interval) {
      return 1;
    } else if (v1->position < v2->position) {
      return -1;
    } else if (v1->position > v2->position) {
      return 1;
    }
    return 0;
  }


} // namespace EVENT_LOGGER
