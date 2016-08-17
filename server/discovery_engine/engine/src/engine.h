//
//  //discovery_engine/engine/engine.h
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//

#ifndef DISCOVERY_ENGINE_ENGINE_H_
#define DISCOVERY_ENGINE_ENGINE_H_

#include "generic.h"
#include "session_db.h"
#include "graph_db.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"

namespace DISCOVERY_ENGINE {
namespace ENGINE {

struct info_record {
  int64_t time;
  int64_t id;
};

// higher time is more recent (LT)
// to make sure values with the same timestamp but different id's are sorted
//    lower to higher id values, this should not occur but better be safe than
//    sorry. Has no performance impact at all as this code will not be reached
//    unless somethings wrong
auto InfoCmp(const MDB_val *val1, const MDB_val *val2) -> int;

class engine {
 public:
  engine(ErrorPtr &e, LMDBEnv &env);
  virtual ~engine(){};

  void beginTransaction(ErrorPtr &e, TransBody body);
  void commitTransaction(ErrorPtr &e, Transaction &txn);

  void handleDiscoverRequest(ErrorPtr &e, int64_t deviceID,
                             DiscoverRequest::Reader request, Transaction &txn);

  void handleSyncResponse(ErrorPtr &e, int64_t deviceID,
                          SyncResponse::Reader response, Transaction &txn);

  void handleDiscoverAckResponse(ErrorPtr &e, int64_t deviceID,
                                 DiscoverAckResponse::Reader, Transaction &txn);

  void onSendSyncRequest(std::function<void(ErrorPtr &e, int64_t deviceID,
                                            SyncRequest::Reader request)>
                             callback);  // register callback
  void onSendDiscoverResponse(std::function<
      void(ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response)>
                                  callback);  // register callback

 private:
  LMDBEnv &env_;
  SessionDB sessionDB_;
  GraphDB graphDB_;
  DatabasePtr<DBVal, DBVal> discoverdDevicesDB_;
  DatabasePtr<DBVal, DBVal> advertisedCards_;
  DatabasePtr<DBVal, DBVal> recentCards_;

  // DeviceDB deviceDB_;

  std::function<void(ErrorPtr &e, int64_t deviceID,
                     SyncRequest::Reader request)> onSendSyncRequest_ = nullptr;

  std::function<void(ErrorPtr &e, int64_t deviceID,
                     DiscoverResponse::Reader response)>
      onSendDiscoverResponse_ = nullptr;

  void handleDeactivateDevice(ErrorPtr &e, int64_t deviceID,
                              DiscoverInfo::Reader request,
                              SyncInfo::Reader syncInfo, Transaction &txn);
  void handleDiscoverCards(ErrorPtr &e, int64_t deviceID,
                           DiscoverInfo::Reader discInfo,
                           SyncInfo::Reader syncInfo, Transaction &txn);
  void processCachedResponse(ErrorPtr &e, int64_t deviceID, int64_t timeStamp,
                             Transaction &txn);
  void processAckResponseSection(ErrorPtr &e, int64_t deviceID,
                                 DiscoverResponse::Reader response,
                                 TimestampSection::Reader section,
                                 Transaction &txn);
  void sendSyncRequest(ErrorPtr &e, int64_t deviceID,
                       SyncRequest::Reader request);
  void sendDiscoverResponse(ErrorPtr &e, int64_t deviceID,
                            DiscoverResponse::Reader response);

};  // class engine

}  // namespace ENGINE

}  // namespace DISCOVERY_ENGINE

#endif  // DISCOVERY_ENGINE_ENGINE_H_
