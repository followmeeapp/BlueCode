//
//  //event_logger/engine/engine.h
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//

#ifndef DASHBOARD_SERVER_ENGINE_H_
#define DASHBOARD_SERVER_ENGINE_H_

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "cpp_lmdb/lmdb_env.h"
#include "rapidjson/document.h"

#include "schema/trace.capnp.h"
#include <string>

namespace DASHBOARD_SERVER {
namespace ENGINE {

using namespace LMDB;
using boost::optional;
using boost::none;
using namespace rapidjson;

struct info_record {
  int64_t time;
  int64_t id;
};

static std::string DEFAULT_DB_PATH =
    "/Users/erik/Documents/Suiron/Data/event_logger";
static std::string DEFAULT_LOG_DB_PATH = DEFAULT_DB_PATH + "/log";
static std::string DEFAULT_META_DB_PATH = DEFAULT_DB_PATH + "/meta";
static size_t DEFAULT_META_DB_SIZE = 4096;

struct Settings {
  std::string dbPath = DEFAULT_DB_PATH;
  std::string logDBPath = DEFAULT_LOG_DB_PATH;
  std::string metaDBPath = DEFAULT_META_DB_PATH;
  size_t metaDBSize = DEFAULT_META_DB_SIZE;
};

class Engine {
 public:
  Engine(ErrorPtr &e);
  Engine(ErrorPtr &e, std::string dbPath, std::string logDBPath,
         std::string metaDBPath, size_t metaDBSize);
  virtual ~Engine(){};

  auto handleGetRequestIDs(ErrorPtr &e, Document &request) -> Document;
  auto handleGetTrailRequest(ErrorPtr &e, Document &request) -> Document;

 private:
  std::shared_ptr<LMDBEnv> env_;
  Settings settings_;
  DatabasePtr<DBVal, DBVal> metaDB_;
  DatabasePtr<DBVal, DBVal> requestUUIDDB_;

  void addTrailJSON(ErrorPtr &e, uint64_t requestID, Document &response,
                    Transaction &txn);

};  // class engine

}  // namespace ENGINE
}  // namespace DASHBOARD_SERVER

#endif  // DASHBOARD_SERVER_ENGINE_H_
