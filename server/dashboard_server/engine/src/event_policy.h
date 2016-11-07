//
//  event_policy.h
//  event_logger
//
//  Created by Erik van der Tier on 26/08/16.
//
//

#ifndef DASHBOARD_SERVER_EVENT_POLICY_H
#define DASHBOARD_SERVER_EVENT_POLICY_H

#include <vector>

#include "generic.h"
#include "schema/trace.capnp.h"

namespace DASHBOARD_SERVER {
namespace ENGINE {

class EventPolicy {
 private:
  std::map<Trace::Event::Type, std::string> TypeStings = {
      {Trace::Event::Type::BEGIN, "BEGIN"},
      {Trace::Event::Type::END, "END"},
      {Trace::Event::Type::THROW, "THROW"}};
  std::map<Trace::Event::Function, std::string> FunctionStrings = {
      {Trace::Event::Function::DECRYPT_MESSAGE, "DECRYPT_MESSAGE"},
      {Trace::Event::Function::ENCRYPT_MESSAGE, "ENCRYPT_MESSAGE"},
      {Trace::Event::Function::FINALIZE_ERROR_RESPONSE,
       "FINALIZE_ERROR_RESPONSE"},
      {Trace::Event::Function::FINALIZE_RESPONSE, "FINALIZE_RESPONSE"},
      {Trace::Event::Function::HELLO_REQUEST, "HELLO_REQUEST"},
      {Trace::Event::Function::INVALID_REQUEST, "INVALID_REQUEST"},
      {Trace::Event::Function::PREPARE_RESPONSE, "PREPARE_RESPONSE"},
      {Trace::Event::Function::PREPARE_TRACE, "PREPARE_TRACE"},
      {Trace::Event::Function::READ_REQUEST, "READ_REQUEST"},
      {Trace::Event::Function::SEND_RESPONSE, "SEND_RESPONSE"},
      {Trace::Event::Function::SERVER_ON_MESSAGE_HANDLER,
       "SERVER_ON_MESSAGE_HANDLER"},
      {Trace::Event::Function::UNKNOWN, "UNKNOWN"},
  };

 public:
  using fields_t = Trace::Event::Reader;

 protected:
  auto makeValues(const fields_t &values)
      -> std::pair<std::vector<const char *>, std::vector<std::uint64_t>>;
};

}  // namespace ENGINE
}  // namespace DASHBOARD_SERVER

#endif /* DASHBOARD_SERVER_EVENT_POLICY_H */
