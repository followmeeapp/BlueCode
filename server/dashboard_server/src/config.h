//
//  config.h
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <string>

namespace DASHBOARD_SERVER {
namespace CONFIG {

const static std::string DEFAULT_DB_PATH =
    "/Users/erik/Documents/Suiron/Data/event_logger";
const static std::string DEFAULT_META_DB_PATH = DEFAULT_DB_PATH + "/meta";
const static int32_t DEFAULT_META_DB_SIZE = 4096;
const static std::string DEFAULT_LOG_DB_PATH = DEFAULT_DB_PATH + "/log";

const static int32_t DEFAULT_SERVER_PORT = 3010;
}
}
