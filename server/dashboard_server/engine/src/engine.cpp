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

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"
#include "traildb-cpp/TrailDBConstructor.h"

#include "schema/trace.capnp.h"

namespace DASHBOARD_SERVER {
namespace ENGINE {

// higher time is more recent (LT)
// to make sure values with the same timestamp but different id's are sorted
//    lower to higher id values, this should not occur but better be safe than
//    sorry. Has no performance impact at all as this code will not be reached
//    unless somethings wrong
auto InfoCmp(const MDB_val *val1, const MDB_val *val2) -> int {
  /*
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
   */
  return 0;
}

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

  metaDB_ = env_->openDatabase<DBVal, DBVal>(e, "meta", MDB_INTEGERKEY);
  ON_ERROR_RETURN(e);
  requestUUIDDB_ =
      env_->openDatabase<DBVal, DBVal>(e, "request_uuid", MDB_INTEGERKEY);
  ON_ERROR_RETURN(e);
}

auto Engine::handleGetRequestIDs(ErrorPtr &e, Document &request) -> Document {
  ASSERT_ERROR_RESET(e);
  LOG("Start handleGetTrailRequest")
  auto &allocator = request.GetAllocator();
  
  // set up response document
  auto Response = Document(Type::kObjectType);
  
  
  
}

auto Engine::handleGetTrailRequest(ErrorPtr &e, Document &request) -> Document {
  ASSERT_ERROR_RESET(e);
  LOG("Start handleGetTrailRequest")
  auto &allocator = request.GetAllocator();

  // set up response document
  auto Response = Document(Type::kObjectType);

  Response.AddMember("response", Value("get-trail"), allocator);

  // check for response ids
  if (!request.HasMember("request-ids")) {
    ERROR_SET_CODE_RETURN_VAL(e, ERROR_CODE::MISSING_REQUEST_IDS_ARRAY,
                              Response);
  }
  auto &RequestIDArray = request["request-ids"];
  if (!RequestIDArray.IsArray()) {
    ERROR_SET_CODE_RETURN_VAL(e, ERROR_CODE::ILLEGAL_OBJECT_TYPE, Response);
  }
  Response.AddMember("response-ids", Value(Type::kArrayType), allocator);
  {  // transaction scope
    auto txn = env_->beginTransaction(e, MDB_RDONLY);
    ON_ERROR_RETURN_VAL(e, Response);

    for (auto &requestIDNode : RequestIDArray.GetArray()) {
      if (!requestIDNode.IsInt())
        ERROR_SET_CODE_RETURN_VAL(e, ERROR_CODE::ILLEGAL_OBJECT_TYPE, Response);

      auto RequestID = requestIDNode.GetInt();

      addTrailJSON(e, RequestID, Response, txn);
      ON_ERROR_RETURN_VAL(e, Response);
    }
    env_->commitTransaction(e, txn);
  }  // end transaction scope
  return Response;
}

void Engine::addTrailJSON(ErrorPtr &e, uint64_t requestID, Document &response,
                          Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  LOG("adding trail for requestid: " << requestID)
  Document::AllocatorType &allocator = response.GetAllocator();

  auto &ResponseIDArray = response["response-ids"];
  if (!ResponseIDArray.IsArray())
    ERROR_SET_CODE_VAL(e, ERROR_CODE::ILLEGAL_OBJECT_TYPE, response);
  ON_ERROR_RETURN(e);

  auto RequestUUIDRecord = requestUUIDDB_->getCopy(e, requestID, txn);
  if (!hasVal(RequestUUIDRecord) && !e)
    ERROR_SET_CODE_VAL(e, ERROR_CODE::MISSING_TRAIL_META, response);
  ON_ERROR_RETURN(e);

  auto RequestUUID = dbValAsPtr<char>(*RequestUUIDRecord);

  // get uuid for requestID
  auto RequestMetaRecord = requestUUIDDB_->get(e, requestID, txn);
  if (!hasVal(RequestUUIDRecord))
    ERROR_SET_CODE_VAL(e, ERROR_CODE::MISSING_TRAIL_META, response);
  ON_ERROR_RETURN(e);

  auto RequestMeta = make_reader(*RequestMetaRecord).getRoot<Trace>();
  auto StartTime = RequestMeta.getTimestamp();
  LOG("StartTime of trail: " << StartTime)

  ResponseIDArray.PushBack(Value(requestID), allocator);

  // auto LogDB = TrailDB(settings_.logDBPath);

  ON_ERROR_RETURN(e);
}

}  // namespace ENGINE
}  // namespace DASHBOARD_SERVER
