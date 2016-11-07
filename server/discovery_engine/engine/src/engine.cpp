//
//  //discovery_graph/graph/graph.cpp
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//
#include "engine.h"

namespace DISCOVERY_ENGINE {

namespace ENGINE {

// higher time is more recent (LT)
// to make sure values with the same timestamp but different id's are sorted
//    lower to higher id values, this should not occur but better be safe than
//    sorry. Has no performance impact at all as this code will not be reached
//    unless somethings wrong
auto InfoCmp(const MDB_val *val1, const MDB_val *val2) -> int {
  auto v1 = *static_cast<info_record *>(val1->mv_data);
  auto v2 = *static_cast<info_record *>(val2->mv_data);
  // sorting from high to low
  if (v1.time < v2.time)
    return 1;
  else if (v1.time > v2.time)
    return -1;
  else {  // sorting id's normal order
    if (v1.id < v2.id)
      return -1;
    else if (v1.id > v2.id)
      return 1;
  }
  return 0;
}

}  // namespace ENGINE
}  // namespace DISCOVERY_ENGINE
