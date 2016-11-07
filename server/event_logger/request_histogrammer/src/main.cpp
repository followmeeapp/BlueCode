//
//  main.cpp
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <chrono>

#include "aeron/util/CommandOptionParser.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"

#include "schema/trace.capnp.h"
#include "schema/rpc.capnp.h"

#include "config.h"

#include "event_logger/request_histogrammer/engine/engine.h"

using namespace std;
using namespace std::chrono;
using namespace aeron::util;
using namespace aeron;
using namespace EVENT_LOGGER::REQUEST_HISTOGRAMMER;

std::atomic<bool> running(true);

void sigIntHandler(int param) { running = false; }

static const char optHelp = 'h';

// LMDB options
static const char optDbPath = 'd';
static const char optMetaDbSize = 'm';
static const char optHistogramDbSize = 's';
static const char optMaxHistogramValue = 'v';

static const char optLowerInterval = 'l';
static const char optUpperInterval = 'u';

namespace EVENT_LOGGER {
namespace REQUEST_HISTOGRAMMER {

struct Settings {
  string dbPath = EVENT_LOGGER::REQUEST_HISTOGRAMMER::CONFIG::DEFAULT_DB_PATH;
  int32_t metaDBSize =
      EVENT_LOGGER::REQUEST_HISTOGRAMMER::CONFIG::DEFAULT_META_DB_SIZE;
  int32_t histDBSize =
      EVENT_LOGGER::REQUEST_HISTOGRAMMER::CONFIG::DEFAULT_HISTOGRAM_DB_SIZE;
  int32_t maxHistVal =
      EVENT_LOGGER::REQUEST_HISTOGRAMMER::CONFIG::DEFAULT_MAX_HISTOGRAM_VALUE;
  int32_t lowerInterval =
      EVENT_LOGGER::REQUEST_HISTOGRAMMER::CONFIG::LOWER_INTERVAL;
  int32_t upperInterval =
      EVENT_LOGGER::REQUEST_HISTOGRAMMER::CONFIG::UPPER_INTERVAL;
};

auto parseCmdLine(int argc, char **argv) -> Settings {
  CommandOptionParser cp;
  cp.addOption(CommandOption(optHelp, 0, 0,
                             "                   Displays help information."));

  cp.addOption(CommandOption(optDbPath, 1, 1,
                             "db_path            Directory for db files."));
  cp.addOption(CommandOption(
      optMetaDbSize, 1, 1,
      "meta_db_size       Meta data file size in MB. (Default = 4096MB)"));
  cp.addOption(CommandOption(
      optHistogramDbSize, 1, 1,
      "histogram_db_size  meta data file size in MB. (Default = 4096MB)"));
  cp.addOption(CommandOption(optMaxHistogramValue, 1, 1,
                             "histogram_max_val  maximum histogram value."));
  cp.addOption(CommandOption(optLowerInterval, 1, 1,
                             "lower_interval     lowest interval."));
  cp.addOption(CommandOption(optUpperInterval, 1, 1,
                             "upper_interval     highest interval."));

  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.dbPath = cp.getOption(optDbPath).getParam(0, s.dbPath);
  s.metaDBSize =
      cp.getOption(optMetaDbSize).getParamAsInt(0, 1, INT32_MAX, s.metaDBSize);
  s.histDBSize = cp.getOption(optHistogramDbSize)
                     .getParamAsInt(0, 1, INT32_MAX, s.histDBSize);
  s.maxHistVal = cp.getOption(optMaxHistogramValue)
                     .getParamAsInt(0, 1, INT32_MAX, s.maxHistVal);
  s.lowerInterval = cp.getOption(optLowerInterval)
                        .getParamAsInt(0, 1, INT32_MAX, s.lowerInterval);
  s.upperInterval = cp.getOption(optUpperInterval)
                        .getParamAsInt(0, 1, INT32_MAX, s.upperInterval);

  return s;
}

}  // namespace REQUEST_HISTOGRAMMER
}  // namespace EVENT_LOGGER

using namespace EVENT_LOGGER::REQUEST_HISTOGRAMMER;

auto main(int argc, char **argv) -> int {
  assert(sizeof(unsigned long long) == sizeof(void *) &&
         "Expect sizeof(unsigned long long) == sizeof(void *)");

  try {
    Settings settings = parseCmdLine(argc, argv);

    LOG("Event Logger::request histogrammer")

    LMDB::ErrorPtr e;

    // set up intervals to process

    std::vector<uint64_t> Intervals;
    for (auto interval : CONFIG::intervals) {
      if (interval >= settings.lowerInterval &&
          interval <= settings.upperInterval)
        Intervals.push_back(interval);
    }
    std::vector<uint64_t> IntervalsWindows = {10, 6, 2};

    // set up Histogram engine

    auto HISTEngine = ENGINE::Engine(e, settings.dbPath, settings.metaDBSize,
                                     settings.histDBSize, settings.maxHistVal,
                                     Intervals, IntervalsWindows);
    if (e) {
      cerr << "ERROR: " << e.message() << endl;
      return -1;
    }
    LOG("Succesfully setup engine, starting work loop at interval size: " << Intervals[0])

    // work loop
    do {
      HISTEngine.processHistograms(e);
      if (e) {
        cerr << "ERROR: " << e.message() << endl;
        return -1;
      }

      // wait lowerInterval milliseconds before next cycle
      std::this_thread::sleep_for(std::chrono::milliseconds(Intervals[0]));
    } while (true);

  } catch (CommandOptionException &e) {
    cerr << "ERROR: " << e.what() << endl;
    return -1;
  }

  return 0;
}
