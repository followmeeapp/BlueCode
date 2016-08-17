//
//  SessionDB.cpp
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#include "session_db.h"

namespace DISCOVERY_ENGINE {
namespace ENGINE {

SessionDB::SessionDB(ErrorPtr &e, LMDBEnv &env) : env_(env) {
  sessionDB_ = env_.openDatabase<DBVal, DBVal>(e, "session", MDB_INTEGERKEY);
  ResponseCacheDB_ =
      env_.openDatabase<DBVal, DBVal>(e, "response_cache", MDB_INTEGERKEY);
}

auto SessionDB::getSession(ErrorPtr &e, int64_t deviceID,
                           DiscoverRequest::Reader request, Transaction &txn)
    -> DBVal * {
  ASSERT_ERROR_RESET(e);
  auto Key = DBVal(deviceID);
  auto Session = sessionDB_->get(e, Key, txn);
  ON_ERROR_RETURN_VAL(e, nullptr);

  if (Session == nullptr) {
    capnp::MallocMessageBuilder builder;
    builder.setRoot(request);
    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    size_t size = bytes.size();
    char *data = (char *)(bytes.begin());

    sessionDB_->add(e, deviceID, DBVal(size, data), txn, false);
    ON_ERROR_RETURN_VAL(e, nullptr);
  }
  return Session;
}

void SessionDB::updateSession(ErrorPtr &e, int64_t deviceID,
                              DiscoverRequest::Reader session,
                              Transaction &txn) {
  ASSERT_ERROR_RESET(e);

  capnp::MallocMessageBuilder builder;
  builder.setRoot(session);
  kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
  kj::ArrayPtr<kj::byte> bytes = words.asBytes();
  size_t size = bytes.size();
  char *data = (char *)(bytes.begin());
  auto Key = DBVal(deviceID);

  sessionDB_->add(e, Key, DBVal(size, data), txn, true);
}

void SessionDB::cacheResponse(ErrorPtr &e, int64_t deviceID,
                              DiscoverResponse::Reader response,
                              Transaction &txn) {
  ASSERT_ERROR_RESET(e);

  capnp::MallocMessageBuilder builder;
  builder.setRoot(response);
  kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
  kj::ArrayPtr<kj::byte> bytes = words.asBytes();
  size_t size = bytes.size();
  char *data = (char *)(bytes.begin());
  auto Key = DBVal(deviceID);

  ResponseCacheDB_->add(e, Key, DBVal(size, data), txn, true);
}

auto SessionDB::getResponse(ErrorPtr &e, int64_t deviceID, Transaction &txn)
    -> DBVal * {
  ASSERT_ERROR_RESET(e);
  auto Key = DBVal(deviceID);
  auto Response = ResponseCacheDB_->get(e, Key, txn);
  ON_ERROR_RETURN_VAL(e, nullptr);

  return Response;
}

void SessionDB::deleteResponse(ErrorPtr &e, int64_t deviceID,
                               DiscoverResponse::Reader response,
                               Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  auto Key = DBVal(deviceID);
  ResponseCacheDB_->del(e, Key, txn);
  ON_ERROR_RETURN(e);
}

}  // namespace ENGINE

}  // namespace DISCOVERY_ENGINE
