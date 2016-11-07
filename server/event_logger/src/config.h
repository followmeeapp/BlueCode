//
//  config.h
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <string>

using namespace std;

namespace EVENT_LOGGER {
namespace CONFIG {

const static int32_t DEFAULT_DB_SIZE = 4096;

const static string DEFAULT_SERVER_CHANNEL =
    "aeron:udp?endpoint=localhost:40125";
const static string DEFAULT_LOGGER_CHANNEL =
    "aeron:udp?endpoint=localhost:40126";
const static int32_t DEFAULT_SERVER_STREAM_ID = 10;
const static int32_t DEFAULT_LOGGER_STREAM_ID = 10;
const static int DEFAULT_LINGER_TIMEOUT_MS = 5000;
const static int DEFAULT_FRAGMENT_COUNT_LIMIT = 10;
}
}
