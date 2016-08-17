//
//  SessionDB.cpp
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#include "graph_db.h"

namespace DISCOVERY_ENGINE {
namespace ENGINE {

GraphDB::GraphDB(ErrorPtr &e, LMDBEnv &env)
    : graphDB_(lmdb_graph<int64_t, int64_t>(e, env)) {}

void GraphDB::update(ErrorPtr &e, int64_t deviceID, int64_t time,
                     capnp::List<DeviceInfo>::Reader discoveredDevices,
                     Transaction &txn) {
  ASSERT_ERROR_RESET(e);

  // create or update the vertex representing our deviceID
  //   on update only the timestamp is set to the current time at sync
  graphDB_.addVertex(e, deviceID, time, txn);
  for (auto discoveredDevice : discoveredDevices) {
    auto DiscDevID = discoveredDevice.getId();
    // add or update linked devices (with timestamp of device discovery)
    graphDB_.addVertex(e, DiscDevID, discoveredDevice.getTimestamp(), txn);
    ON_ERROR_RETURN(e);

    // add or update edge between deviceID and discovered device
    // check on existing edge (in or out)
    auto ExistingEdge = graphDB_.getEdge(e, deviceID, DiscDevID, txn);
    if (ExistingEdge.is_initialized()) {
      graphDB_.addEdge(e, deviceID, DiscDevID, discoveredDevice.getTimestamp(),
                       txn);
    }
    ON_ERROR_RETURN(e);
    continue;
    ExistingEdge = graphDB_.getEdge(e, DiscDevID, deviceID, txn);
    if (ExistingEdge.is_initialized()) {
      graphDB_.addEdge(e, DiscDevID, deviceID, discoveredDevice.getTimestamp(),
                       txn);
    }
    ON_ERROR_RETURN(e);
    continue;
    graphDB_.addEdge(e, deviceID, DiscDevID, discoveredDevice.getTimestamp(),
                     txn);
    ON_ERROR_RETURN(e);
  }  // end for (auto discoveredDevice : discoveredDevices)
}

auto GraphDB::discoverDevices(ErrorPtr &e, int64_t deviceID, Transaction &txn)
    -> std::vector<int64_t> {
  ASSERT_ERROR_RESET(e);

  std::map<int64_t, int> DeviceDepth;
  std::vector<int64_t> FoundDevices;
  auto RootDevice = graphDB_.getVertex(e, deviceID, txn);

  if (!hasVal(RootDevice)) return FoundDevices;
  // we have a vertex for our device

  int depth = 0;
  int position = 0;
  LOG("\nDiscovery Devices in Graph!")
  // We put our 'RootDevice' in the FoundDevices
  FoundDevices.push_back(*RootDevice);
  DeviceDepth[*RootDevice] = depth;
  while (depth < 3) {
    auto size = FoundDevices.size();
    for (auto it = position; it < size; ++it) {
      graphDB_.forEachEdge(
          e, FoundDevices[it], txn,
          [&FoundDevices, &DeviceDepth, &depth, &position, &deviceID, it](
              ErrorPtr &e, const uint64_t *vertexID1, const uint64_t *vertexID2,
              const DBVal *properties, LMDB::Transaction &txn) {
            auto vertexID =
                (*vertexID1 == FoundDevices[it]) ? *vertexID2 : *vertexID1;
            LOG("Found: " << vertexID << " at depth: " << depth)
            if (DeviceDepth.find(vertexID) != DeviceDepth.end()) return;
            LOG("Keeping: " << vertexID << " at depth: " << depth)
            DeviceDepth[vertexID] = depth;
            FoundDevices.push_back(vertexID);
          });
    }  // end for (auto it = position; it < size; ++it)
    position = size;
    if (position >= FoundDevices.size())
      break;  // if we're going past the size we're done.
    depth++;
  }  // end while (depth < 3)

  // remove the root device of the graph (our 'own' device).
  FoundDevices.erase(FoundDevices.begin());

  return FoundDevices;
}

void GraphDB::clean(ErrorPtr &e, int64_t deviceID, int64_t time,
                    Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  //
  auto DiscoveredDevices = graphDB_.getEdges(e, deviceID, txn);

  std::for_each(DiscoveredDevices.cbegin(), DiscoveredDevices.cend(),
                [this, &e, &time, &deviceID, &txn](auto DeviceEdge) {
                  if (DeviceEdge.properties <= time)
                    graphDB_.removeEdge(e, DeviceEdge.vertexID1,
                                        DeviceEdge.vertexID2, txn);
                  auto OtherDeviceID = deviceID == DeviceEdge.vertexID1
                                           ? DeviceEdge.vertexID2
                                           : DeviceEdge.vertexID1;
                  auto Device = graphDB_.getVertex(e, OtherDeviceID, txn);
                  if (Device.is_initialized()) {
                    if (graphDB_.getDegree(e, OtherDeviceID, txn) > 0 &&
                        DeviceEdge.properties <= time)
                      graphDB_.removeVertex(e, OtherDeviceID, txn);
                  }
                  ON_ERROR_RETURN(e);
                });

  // add removal of device itself
  graphDB_.removeVertex(e, deviceID);
}

}  // namespace ENGINE

}  // namespace DISCOVERY_ENGINE
