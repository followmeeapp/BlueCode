//
//  config.h
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <string>

using namespace std;

namespace EVENT_LOGGER {
namespace REQUEST_HISTOGRAMMER {
namespace CONFIG {

const static string DEFAULT_DB_PATH = "";
const static int32_t DEFAULT_META_DB_SIZE = 4096;
const static int32_t DEFAULT_HISTOGRAM_DB_SIZE = 4096;
const static int32_t DEFAULT_MAX_HISTOGRAM_VALUE = 1000 * 60;

const static int LOWER_INTERVAL = 1000;
const static int UPPER_INTERVAL = 1000 * 60 * 10;

const static int intervals[] = {1000, 1000 * 60, 1000 * 60 * 10};
}
}
}
