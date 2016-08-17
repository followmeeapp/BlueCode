//
//  config.h
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <string>

using namespace std;

namespace DISCOVERY_ENGINE {
namespace CONFIG {

const static int32_t DEFAULT_DB_SIZE              = 4096;

const static string  DEFAULT_SERVER_CHANNEL       = "aeron:udp?endpoint=localhost:40123";
const static string  DEFAULT_ENGINE_CHANNEL       = "aeron:udp?endpoint=localhost:40124";
const static int32_t DEFAULT_SERVER_STREAM_ID     = 10;
const static int32_t DEFAULT_ENGINE_STREAM_ID     = 10;
const static int     DEFAULT_LINGER_TIMEOUT_MS    = 5000;
const static int     DEFAULT_FRAGMENT_COUNT_LIMIT = 10;

}}
