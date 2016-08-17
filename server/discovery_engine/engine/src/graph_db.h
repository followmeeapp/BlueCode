//
//  graph_db.hpp
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#ifndef DISCOVERY_ENGINE_GRAPHDB_H_
#define DISCOVERY_ENGINE_GRAPHDB_H_

#include "generic.h"
#include "discovery_engine/xy_lmdb_graph/xy_lmdb_graph.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"

using namespace XYGraphEngine;

namespace DISCOVERY_ENGINE {
namespace ENGINE {

class GraphDB {
 public:
  GraphDB(ErrorPtr &e, LMDBEnv &env);
  virtual ~GraphDB(){};

  void update(ErrorPtr &e, int64_t deviceID, int64_t time,
              capnp::List<DeviceInfo>::Reader devices, Transaction &txn);

  auto discoverDevices(ErrorPtr &e, int64_t deviceID, Transaction &txn)
      -> std::vector<int64_t>;

  void clean(ErrorPtr &e, int64_t deviceID, int64_t time, Transaction &txn);

 private:
  lmdb_graph<int64_t, int64_t> graphDB_;
};

}  // namespace ENGINE

}  // namespace DISCOVERY_ENGINE

#endif  // DISCOVERY_ENGINE_GRAPHDB_H_
