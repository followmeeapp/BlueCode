//
//  generic.h
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#ifndef generic_h
#define generic_h

#include <chrono>
#include "boost/optional.hpp"
#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "discovery_engine/cpp_lmdb/lmdb_database.h"
#include "discovery_engine/cpp_lmdb/lmdb_env.h"
#include "discovery_engine/cpp_lmdb/lmdb_cursor.h"

using boost::optional;
using boost::none;
using namespace std::chrono;
using namespace LMDB;

#ifndef NDEBUG
#include <iostream>
#define LOG(message)                                                     \
  std::cout << __TIME__ << ": " << __FILE__ << ", " << __LINE__ << ":\n" \
            << "   " << message << "\n";
#else
#define LOG(message)
#endif

// ______________________________________
// helper functions for optionals

// returns true if the optional has a value
template <typename T>
inline auto hasVal(T optional) -> bool {
  return optional.is_initialized();
}

// emmulates swift's 'if let', returns true if the optional has a value
// unwraps the optional and executes the closure with it
template <typename T>
inline auto ifLet(boost::optional<T> optional,
                  std::function<void(T unwrapped)> body) -> bool {
  if (!hasVal(optional)) return false;

  body(*optional);
  return true;
}

//________________________________________

using TransBody = std::function<void(ErrorPtr &, Transaction &)>;

auto inline now() -> int64_t {
  auto TimeSinceEpoch = system_clock::now().time_since_epoch().count();
  return milliseconds(TimeSinceEpoch).count();
}

auto inline make_reader(DBVal data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 &&
         "expected data.mv_size to be a multiple of 8");

  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data,
                                       data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}

namespace DISCOVERY_ENGINE {
namespace ENGINE {
enum ERROR_CODE {
  UNKNOWN_DISC_TYPE = -50000,
  MISSING_ON_SEND_SYNC_REQUEST_HANLDER,
  MISSING_ON_SEND_DISCOVER_RESPONSE_HANLDER,
  MISSING_RESPONSE_CACHE,
  MISMATCH_RESPONSE_ACK_TIMESTAMP,
  INCORRECT_SYNC_ON_RESPONSE,
};
static std::map<int, std::string> engine_errors = {
    {ERROR_CODE::UNKNOWN_DISC_TYPE, "discovery type is unknown"},
    {ERROR_CODE::MISSING_ON_SEND_SYNC_REQUEST_HANLDER,
     "missing onSendSyncRequest handler"},
    {ERROR_CODE::MISSING_ON_SEND_DISCOVER_RESPONSE_HANLDER,
     "missing onSendDiscoverResponse handler"},
    {ERROR_CODE::MISSING_RESPONSE_CACHE, "missing cached response"},
    {ERROR_CODE::MISMATCH_RESPONSE_ACK_TIMESTAMP,
     "Mismatch between Ack timestamp and response timestamp"},
    {ERROR_CODE::INCORRECT_SYNC_ON_RESPONSE, "We're incorrectly out of sync"},
};

struct engine_error_t : LMDB::error_t {
  std::string message() override;
  engine_error_t(int code);
};
}  // namespace ENGINE
}  // namespace DISCOVERY_ENGINE

#endif /* generic_h */
