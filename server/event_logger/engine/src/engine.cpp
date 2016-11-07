//
//  //event_logger/graph/graph.cpp
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//
#include "engine.h"
#include "event_policy.h"
#include <uuid/uuid.h>
#include "generic.h"
#include "event_logger/event_logger.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"
#include "traildb-cpp/TrailDBConstructor.h"

#include "schema/trace.capnp.h"
using namespace EVENT_LOGGER;

namespace EVENT_LOGGER {
namespace ENGINE {

Engine::Engine(ErrorPtr &e)
    : Engine(e, DEFAULT_DB_PATH, DEFAULT_LOG_DB_PATH, DEFAULT_META_DB_PATH,
             DEFAULT_META_DB_SIZE) {}

Engine::Engine(ErrorPtr &e, std::string dbPath, std::string logDBPath,
               std::string metaDBPath, size_t metaDBSize) {
  ASSERT_ERROR_RESET(e);
  settings_.dbPath = dbPath;
  settings_.logDBPath = logDBPath;
  settings_.metaDBPath = metaDBPath;
  settings_.metaDBSize = metaDBSize;

  env_ = std::make_shared<LMDB::LMDBEnv>(e, metaDBPath, 16, metaDBSize);
  ON_ERROR_RETURN(e);

  metaDB_ = env_->openDatabase<DBVal, DBVal>(e, "meta", 0);
  ON_ERROR_RETURN(e);
  metaDB_->setKeyCompareFunction(e, TraceKeyCmp);
  ON_ERROR_RETURN(e);

  requestUUIDDB_ =
      env_->openDatabase<DBVal, DBVal>(e, "request_uuid", MDB_INTEGERKEY);
  ON_ERROR_RETURN(e);
}

void Engine::logTrail(ErrorPtr &e, Trace::Reader trace) {
  ASSERT_ERROR_RESET(e);
  LOG("BEGIN: logTrail")
  capnp::MallocMessageBuilder builder;

  auto RequestID = trace.getRequestId();
  uuid_t id;
  char *StringUUID = new char[37];
  auto TraceTimeStamp = trace.getTimestamp();

  // record trace events in metaDB
  std::vector<std::string> Fields{"Type", "Function"};

  LOG("before constructing trail")
  TrailDBConstructor<EventPolicy> TrailDB(settings_.logDBPath, Fields);
  LOG("after constructing trail")

  for (auto event : trace.getEvents()) {
    TrailDB.Add(StringUUID, event.getTimestamp() - TraceTimeStamp, event);
  }
  TrailDB.Finalize();

  // record trace in metaDB

  {  // transaction scope
    auto txn = env_->beginTransaction(e);
    ON_ERROR_RETURN(e);

    uuid_generate(id);
    uuid_unparse(id, StringUUID);
    LOG("Generated UUID: " << StringUUID)
    auto UUIDRecord = DBVal(37, StringUUID);
    requestUUIDDB_->add(e, RequestID, UUIDRecord, txn, false);
    ON_ERROR_RETURN(e);

    auto TraceReq = builder.initRoot<Trace>();
    TraceReq.setTimestamp(TraceTimeStamp);
    TraceReq.setDeviceId(trace.getDeviceId());
    TraceReq.setRequestId(trace.getRequestId());
    TraceReq.setDuration(trace.getDuration());
    TraceReq.setError(trace.getError());
    TraceReq.setStackTrace(trace.getStackTrace());
    TraceReq.setRequestData(trace.getRequestData());
    auto NumTraces = trace.getEvents().size();
    if (NumTraces == 0) {
      e.set(new engine_error_t(ERROR_CODE::MISSING_TRACE_DATA));
    }

    auto RequestType = trace.getEvents()[0].getFunction();

    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();

    auto MetaRecord = DBVal(bytes.size(), bytes.begin());

    trace_key TraceKey = {TraceTimeStamp, static_cast<int16_t>(RequestType)};
    metaDB_->add(e, TraceKey, MetaRecord, txn, false);
    ON_ERROR_RETURN(e);

    env_->commitTransaction(e, txn);
    ON_ERROR_RETURN(e);
  }  // end transaction scope
}

}  // namespace ENGINE
}  // namespace EVENT_LOGGER
