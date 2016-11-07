//
//  generic.h
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#ifndef DASHBOARD_SERVER_ENGINE_GENERIC_H
#define DASHBOARD_SERVER_ENGINE_GENERIC_H

#include <chrono>
#include "boost/optional.hpp"
#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "cpp_lmdb/lmdb_database.h"
#include "cpp_lmdb/lmdb_env.h"
#include "cpp_lmdb/lmdb_cursor.h"

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

auto inline make_reader(DBVal &data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 &&
         "expected data.mv_size to be a multiple of 8");

  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data,
                                       data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}

namespace DASHBOARD_SERVER {
namespace ENGINE {
enum ERROR_CODE {
  MISSING_TRAIL_META = -60000,
  MISSING_RESPONSE_IDS_ARRAY,
  MISSING_REQUEST_IDS_ARRAY,
  ILLEGAL_OBJECT_TYPE,
  MISSING_RESPONSE_CACHE,
  MISMATCH_RESPONSE_ACK_TIMESTAMP,
  INCORRECT_SYNC_ON_RESPONSE,
};
static std::map<int, std::string> engine_errors = {
    {ERROR_CODE::MISSING_TRAIL_META, "missing trail meta"},
    {ERROR_CODE::MISSING_RESPONSE_IDS_ARRAY, "missing response id's array"},
    {ERROR_CODE::MISSING_REQUEST_IDS_ARRAY, "missing request id's array"},
    {ERROR_CODE::ILLEGAL_OBJECT_TYPE, "missing onSendDiscoverResponse handler"},
    {ERROR_CODE::MISSING_RESPONSE_CACHE, "missing cached response"},
    {ERROR_CODE::MISMATCH_RESPONSE_ACK_TIMESTAMP,
     "Mismatch between Ack timestamp and response timestamp"},
    {ERROR_CODE::INCORRECT_SYNC_ON_RESPONSE, "We're incorrectly out of sync"},
};

struct engine_error_t : LMDB::error_t {
  std::string message() override;
  engine_error_t(int code);
};

#define ERROR_SET_CODE(e, code) e.set(new engine_error_t(code));

#define ERROR_SET_CODE_VAL(e, code, val)                      \
  {                                                           \
    e.set(new engine_error_t(code));                          \
    val.AddMember("error", Value(engine_errors[code].c_str(), \
                                 engine_errors[code].size()), \
                  val.GetAllocator());                        \
  }

#define ERROR_SET_CODE_RETURN(e, code) \
  {                                    \
    e.set(new engine_error_t(code));   \
    return;                            \
  }
#define ERROR_SET_CODE_VAL_RETURN(e, code, val)               \
  {                                                           \
    e.set(new engine_error_t(code));                          \
    val.AddMember("error", Value(engine_errors[code].c_str(), \
                                 engine_errors[code].size()), \
                  val.GetAllocator());                        \
    return;                                                   \
  }
#define ERROR_SET_CODE_RETURN_VAL(e, code, val)               \
  {                                                           \
    e.set(new engine_error_t(code));                          \
    val.AddMember("error", Value(engine_errors[code].c_str(), \
                                 engine_errors[code].size()), \
                  val.GetAllocator());                        \
    return val;                                               \
  }

#define ERROR_RETURN_VAL(e, val)                              \
  {                                                           \
    val.AddMember("error", Value(engine_errors[code].c_str(), \
                                 engine_errors[code].size()), \
                  val.GetAllocator());                        \
    return val;                                               \
  }

#define ON_ERROR_RETURN_CODE(e, code) \
  if (e) {                            \
    e.set(new engine_error_t(code));  \
    return;                           \
  }

#define ON_ERROR_SET_CODE(e, code) \
  if (e) e.set(new engine_error_t(code))

}  // namespace ENGINE
}  // namespace DASHBOARD_SERVER

#endif /* DASHBOARD_SERVER_ENGINE_GENERIC_H */
