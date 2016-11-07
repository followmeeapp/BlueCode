//
//  generic.cpp
//  engine
//
//  Created by Erik van der Tier on 28/07/16.
//
//

#include "generic.h"

namespace EVENT_LOGGER {
namespace ENGINE {

std::string engine_error_t::message() {
  if (getCode() > MDB_LAST_ERRCODE)
    return mdb_strerror(code);
  else
    return engine_errors[code].c_str();
};

engine_error_t::engine_error_t(int code) : error_t(code){};

}  // namespace ENGINE
}  // namespace EVENT_LOGGER