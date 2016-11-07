//
//  generic.h
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#ifndef EVENT_LOGGER_ENGINE_GENERIC_H
#define EVENT_LOGGER_ENGINE_GENERIC_H

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

namespace EVENT_LOGGER {
namespace ENGINE {
enum ERROR_CODE {
  MISSING_TRACE_DATA = -50000,
};
static std::map<int, std::string> engine_errors = {
    {ERROR_CODE::MISSING_TRACE_DATA, "Missing trace data in logging attempt"},

};

struct engine_error_t : LMDB::error_t {
  std::string message() override;
  engine_error_t(int code);
};
}  // namespace ENGINE
}  // namespace EVENT_LOGGER

#endif /* EVENT_LOGGER_ENGINE_GENERIC_H */
