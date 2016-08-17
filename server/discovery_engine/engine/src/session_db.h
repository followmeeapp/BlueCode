//
//  SessionDB.hpp
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#ifndef DISCOVERY_ENGINE_SESSIONDB_H_
#define DISCOVERY_ENGINE_SESSIONDB_H_

#include "generic.h"
#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"

using namespace LMDB;

namespace DISCOVERY_ENGINE {
namespace ENGINE {

class SessionDB {
 public:
  SessionDB(ErrorPtr &e, LMDBEnv &env);
  virtual ~SessionDB(){};

  auto getSession(ErrorPtr &e, int64_t deviceID,
                  DiscoverRequest::Reader request, Transaction &txn) -> DBVal *;
  void updateSession(ErrorPtr &e, int64_t deviceID,
                     DiscoverRequest::Reader session, Transaction &txn);

  void cacheResponse(ErrorPtr &e, int64_t deviceID,
                     DiscoverResponse::Reader response, Transaction &txn);

  auto getResponse(ErrorPtr &e, int64_t deviceID, Transaction &txn) -> DBVal *;

  void deleteResponse(ErrorPtr &e, int64_t deviceID,
                      DiscoverResponse::Reader response, Transaction &txn);

 private:
  LMDBEnv &env_;
  DatabasePtr<DBVal, DBVal> sessionDB_;
  DatabasePtr<DBVal, DBVal> ResponseCacheDB_;
};

}  // namespace ENGINE

}  // namespace DISCOVERY_ENGINE

#endif  // DISCOVERY_ENGINE_SESSIONDB_H_
