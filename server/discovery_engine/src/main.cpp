//
//  main.cpp
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include "config.h"
#include "discoveryd.h"
#include "clock/clock.h"

using namespace aeron;

static const char optHelp = 'h';

// LMDB options
static const char optDbPath = 'd';
static const char optDbSize = 'm';

// Aeron options
static const char optPrefix = 'p';
static const char optServerChannel = 'c';
static const char optEngineChannel = 'C';
static const char optServerStreamId = 's';
static const char optEngineStreamId = 'S';
static const char optFrags = 'f';

std::atomic<bool> running(true);
void sigIntHandler(int param) { running = false; }

void parseCmdLine(DiscoverydSettings::Builder settings,
                  aeron::CommandOptionParser &cp, int argc, char **argv) {
  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  using namespace DISCOVERY_ENGINE::CONFIG;

  settings.setDbPath(cp.getOption(optDbPath).getParam(0, DEFAULT_DB_PATH));
  settings.setDbSize(
      cp.getOption(optDbSize).getParamAsInt(0, 1, INT32_MAX, DEFAULT_DB_SIZE));

  settings.setDirPrefix(
      cp.getOption(optPrefix).getParam(0, DEFAULT_DIR_PREFIX));
  settings.setServerChannel(
      cp.getOption(optServerChannel).getParam(0, DEFAULT_SERVER_CHANNEL));
  settings.setEngineChannel(
      cp.getOption(optEngineChannel).getParam(0, DEFAULT_ENGINE_CHANNEL));
  settings.setServerStreamId(
      cp.getOption(optServerStreamId)
          .getParamAsInt(0, 1, INT32_MAX, DEFAULT_SERVER_STREAM_ID));
  settings.setEngineStreamId(
      cp.getOption(optEngineStreamId)
          .getParamAsInt(0, 1, INT32_MAX, DEFAULT_ENGINE_STREAM_ID));
  settings.setFragmentCountLimit(cp.getOption(optFrags).getParamAsInt(
      0, 1, INT32_MAX, DEFAULT_FRAGMENT_COUNT_LIMIT));
}

auto main(int argc, char **argv) -> int {
  assert(sizeof(unsigned long long) == sizeof(void *) &&
         "Expect sizeof(unsigned long long) == sizeof(void *)");

  aeron::CommandOptionParser cp;
  cp.addOption(CommandOption(optHelp, 0, 0,
                             "                Displays help information."));

  cp.addOption(CommandOption(optDbPath, 1, 1,
                             "db_path         Directory for LMDB data."));
  cp.addOption(CommandOption(
      optDbSize, 1, 1,
      "db_size         LMDB data file size in MB. (Default = 4096MB)"));

  cp.addOption(CommandOption(
      optPrefix, 1, 1, "dir             Prefix directory for aeron driver."));
  cp.addOption(
      CommandOption(optServerChannel, 1, 1, "channel         Server Channel."));
  cp.addOption(
      CommandOption(optEngineChannel, 1, 1, "channel         Engine Channel."));
  cp.addOption(CommandOption(optServerStreamId, 1, 1,
                             "streamId        Server Stream ID."));
  cp.addOption(CommandOption(optEngineStreamId, 1, 1,
                             "streamId        Engine Stream ID."));
  cp.addOption(
      CommandOption(optFrags, 1, 1, "limit           Fragment Count Limit."));

  signal(SIGINT, sigIntHandler);
  try {
    capnp::MallocMessageBuilder SettingsBuilder;

    auto settings = SettingsBuilder.initRoot<DiscoverydSettings>();
    parseCmdLine(settings, cp, argc, argv);

    auto Clock = xy::clock<xy::ClockSource>();

    return runServer(settings, Clock, running);

  } catch (CommandOptionException &e) {
    cerr << "ERROR: " << e.what() << endl
         << endl;
    cp.displayOptionsHelp(cerr);
    return -1;
  }
}
