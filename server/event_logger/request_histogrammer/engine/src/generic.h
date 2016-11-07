//
//  generic.h
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#ifndef EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_GENERIC_H
#define EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_GENERIC_H

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

// emmulates swift's 'if let', returns true if the optional has a value
// unwraps the optional and executes the closure with it
template <typename T>
inline auto ifLet(boost::optional<T> optional,
                  std::function<void(T unwrapped)> body) -> bool {
  if (!optional) return false;

  body(*optional);
  return true;
}

//________________________________________

// using TransBody = std::function<void(ErrorPtr &, Transaction &)>;

auto inline now() -> int64_t {
  auto TimeSinceEpoch = system_clock::now().time_since_epoch().count();
  return milliseconds(TimeSinceEpoch).count();
}

auto inline make_reader(LMDB::DBVal data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 &&
         "expected data.mv_size to be a multiple of 8");

  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data,
                                       data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}

namespace EVENT_LOGGER {
namespace REQUEST_HISTOGRAMMER {
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

#define ON_ERROR_SET_RETURN_CODE(e, code) \
  if (e) {                                \
    e.set(new engine_error_t(code));      \
    return code;                          \
  }

#define ON_ERROR_SET_CODE_RETURN(e, code) \
  if (e) {                                \
    e.set(new engine_error_t(code));      \
    return;                               \
  }

#define ON_ERROR_RETURN_CODE(e) \
  if (e) {                      \
    return e.code();            \
  }

#define ON_ERROR_SET_CODE(e, code) \
  if (e) e.set(new engine_error_t(code))

}  // namespace ENGINE
}  // namespace REQUEST_HISTOGRAMMER
}  // namespace EVENT_LOGGER

#endif /* EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_GENERIC_H */
