//
//  event_policy.cpp
//  engine
//
//  Created by Erik van der Tier on 30/08/16.
//
//

#include "event_policy.h"

namespace DASHBOARD_SERVER {
namespace ENGINE {

auto EventPolicy::makeValues(const fields_t &values)
    -> std::pair<std::vector<const char *>, std::vector<std::uint64_t>> {
  std::vector<const char *> val_entries;
  std::vector<std::uint64_t> val_len;

  auto Type = TypeStings[values.getType()].c_str();
  auto Function = FunctionStrings[values.getFunction()].c_str();

  val_len.push_back(strlen(Type));
  val_len.push_back(strlen(Function));

  val_entries.push_back(Type);
  val_entries.push_back(Function);

  return std::make_pair(val_entries, val_len);
}

}  // namespace ENGINE
}  // namespace DASHBOARD_SERVER
