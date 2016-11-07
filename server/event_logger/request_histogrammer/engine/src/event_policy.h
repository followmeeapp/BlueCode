//
//  event_policy.h
//  event_logger
//
//  Created by Erik van der Tier on 26/08/16.
//
//

#ifndef EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_EVENT_POLICY_H
#define EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_EVENT_POLICY_H

#include <vector>

#include "generic.h"
#include "schema/trace.capnp.h"

namespace EVENT_LOGGER {
namespace REQUEST_HISTOGRAMMER {
namespace ENGINE {

class EventPolicy {
 private:
  std::map<Trace::Event::Type, std::string> TypeStings = {
      {Trace::Event::Type::BEGIN, "BEGIN"},
      {Trace::Event::Type::END, "END"},
      {Trace::Event::Type::THROW, "THROW"}};
  std::map<Trace::Event::Function, std::string> FunctionStrings = {
      {Trace::Event::Function::FINALIZE_ERROR_RESPONSE,
       "FINALIZE_ERROR_RESPONSE"},
      {Trace::Event::Function::FINALIZE_RESPONSE, "FINALIZE_RESPONSE"},
      {Trace::Event::Function::HANDLE_BACKUP_LIST_REQUEST,
       "HANDLE_BACKUP_LIST_REQUEST"},
      {Trace::Event::Function::HANDLE_BACKUP_REQUEST, "HANDLE_BACKUP_REQUEST"},
      {Trace::Event::Function::HANDLE_CARD_REQUEST, "HANDLE_CARD_REQUEST"},
      {Trace::Event::Function::HANDLE_CLIENT_MESSAGE, "HANDLE_CLIENT_MESSAGE"},
      {Trace::Event::Function::HANDLE_CREATE_BACKUP_REQUEST,
       "HANDLE_CREATE_BACKUP_REQUEST"},
      {Trace::Event::Function::HANDLE_CREATE_CARD_REQUEST,
       "HANDLE_CREATE_CARD_REQUEST"},
      {Trace::Event::Function::HANDLE_HELLO_REQUEST, "HANDLE_HELLO_REQUEST"},
      {Trace::Event::Function::HANDLE_INVALID_MESSAGE,
       "HANDLE_INVALID_MESSAGE"},
      {Trace::Event::Function::HANDLE_JOIN_REQUEST, "HANDLE_JOIN_REQUEST"},
      {Trace::Event::Function::HANDLE_REQUEST_NOT_IMPLEMENTED,
       "HANDLE_REQUEST_NOT_IMPLEMENTED"},
      {Trace::Event::Function::HANDLE_UPDATE_CARD_REQUEST,
       "HANDLE_UPDATE_CARD_REQUEST"},
      {Trace::Event::Function::PREPARE_RESPONSE, "PREPARE_RESPONSE"},
      {Trace::Event::Function::PREPARE_TRACE, "PREPARE_TRACE"},
      {Trace::Event::Function::READ_REQUEST, "READ_REQUEST"},
      {Trace::Event::Function::SEND_RESPONSE, "SEND_RESPONSE"},
      {Trace::Event::Function::UNKNOWN, "UNKNOWN"},
  };

 public:
  using fields_t = Trace::Event::Reader;

 protected:
  auto makeValues(const fields_t &values)
      -> std::pair<std::vector<const char *>, std::vector<std::uint64_t>>;
};

}  // namespace ENGINE
}  // namespace REQUEST_HISTOGRAMMER
}  // namespace EVENT_LOGGER

#endif /* EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_EVENT_POLICY_H */
