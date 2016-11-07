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

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"
#include "aeron/util/CommandOptionParser.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"

#include "schema/trace.capnp.h"
#include "schema/rpc.capnp.h"

#include "config.h"

#include "event_logger/engine/engine.h"

using namespace std;
using namespace std::chrono;
using namespace aeron::util;
using namespace aeron;

std::atomic<bool> running(true);

void sigIntHandler(int param) { running = false; }

static const char optHelp = 'h';

// LMDB options
static const char optDbPath = 'd';
static const char optDbSize = 'm';

// Aeron options
static const char optPrefix = 'p';
static const char optServerChannel = 'c';
static const char optLoggerChannel = 'C';
static const char optServerStreamId = 's';
static const char optLoggerStreamId = 'S';
static const char optFrags = 'f';

struct Settings {
  string dbPath = "/Users/erik/Documents/Suiron/Data/event_logger";
  string logDBPath = "/Users/erik/Documents/Suiron/Data/event_logger/log";
  string metaDBPath = "/Users/erik/Documents/Suiron/Data/event_logger/meta";
  int32_t metaDBSize = EVENT_LOGGER::CONFIG::DEFAULT_DB_SIZE;

  string dirPrefix = "/Users/erik/Documents/Suiron/Data/aeron";
  string serverChannel = EVENT_LOGGER::CONFIG::DEFAULT_SERVER_CHANNEL;
  string loggerChannel = EVENT_LOGGER::CONFIG::DEFAULT_LOGGER_CHANNEL;
  int32_t serverStreamId = EVENT_LOGGER::CONFIG::DEFAULT_SERVER_STREAM_ID;
  int32_t loggerStreamId = EVENT_LOGGER::CONFIG::DEFAULT_LOGGER_STREAM_ID;
  int32_t fragmentCountLimit =
      EVENT_LOGGER::CONFIG::DEFAULT_FRAGMENT_COUNT_LIMIT;
};

auto parseCmdLine(int argc, char **argv) -> Settings {
  CommandOptionParser cp;
  cp.addOption(CommandOption(optHelp, 0, 0,
                             "                Displays help information."));

  cp.addOption(CommandOption(optDbPath, 1, 1,
                             "db_path         Directory for traildb data."));
  cp.addOption(CommandOption(
      optDbSize, 1, 1,
      "db_size         LMDB data file size in MB. (Default = 4096MB)"));

  cp.addOption(CommandOption(
      optPrefix, 1, 1, "dir             Prefix directory for aeron driver."));
  cp.addOption(
      CommandOption(optServerChannel, 1, 1, "channel         Server Channel."));
  cp.addOption(
      CommandOption(optLoggerChannel, 1, 1, "channel         Engine Channel."));
  cp.addOption(CommandOption(optServerStreamId, 1, 1,
                             "streamId        Server Stream ID."));
  cp.addOption(CommandOption(optLoggerStreamId, 1, 1,
                             "streamId        Engine Stream ID."));
  cp.addOption(
      CommandOption(optFrags, 1, 1, "limit           Fragment Count Limit."));

  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.logDBPath = cp.getOption(optDbPath).getParam(0, s.logDBPath);
  s.metaDBSize =
      cp.getOption(optDbSize).getParamAsInt(0, 1, INT32_MAX, s.metaDBSize);

  s.dirPrefix = cp.getOption(optPrefix).getParam(0, s.dirPrefix);
  s.serverChannel = cp.getOption(optServerChannel).getParam(0, s.serverChannel);
  s.loggerChannel = cp.getOption(optLoggerChannel).getParam(0, s.loggerChannel);
  s.serverStreamId = cp.getOption(optServerStreamId)
                         .getParamAsInt(0, 1, INT32_MAX, s.serverStreamId);
  s.loggerStreamId = cp.getOption(optLoggerStreamId)
                         .getParamAsInt(0, 1, INT32_MAX, s.loggerStreamId);
  s.fragmentCountLimit = cp.getOption(optFrags).getParamAsInt(
      0, 1, INT32_MAX, s.fragmentCountLimit);

  return s;
}

void messageHandler(EVENT_LOGGER::ENGINE::Engine &engine,
                    std::shared_ptr<Publication> enginePublication,
                    AtomicBuffer &buffer, index_t offset, index_t length) {
  uint8_t *data = buffer.buffer() + offset;
  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data, length / 8);
  auto reader = capnp::FlatArrayMessageReader(view);
  Trace::Reader trace = reader.getRoot<Trace>();
  LMDB::ErrorPtr e;
  engine.logTrail(e, trace);
}

auto main(int argc, char **argv) -> int {
  assert(sizeof(unsigned long long) == sizeof(void *) &&
         "Expect sizeof(unsigned long long) == sizeof(void *)");

  signal(SIGINT, sigIntHandler);

  try {
    Settings settings = parseCmdLine(argc, argv);

    cout << "Event Logger" << endl;
    cout << "Publishing logger at " << settings.loggerChannel
         << " on Stream ID " << settings.loggerStreamId << endl;
    cout << "Subscribing Server at " << settings.serverChannel
         << " on Stream ID " << settings.serverStreamId << endl;

    aeron::Context context;
    int64_t publicationId;
    int64_t subscriptionId;
    std::atomic<bool> shouldPoll(false);

    std::shared_ptr<Publication> loggerPublication = nullptr;
    std::shared_ptr<Subscription> serverSubscription = nullptr;

    if (settings.dirPrefix != "") {
      context.aeronDir(settings.dirPrefix);
    }

    context.newPublicationHandler([](const string &channel, int32_t streamId,
                                     int32_t sessionId, int64_t correlationId) {
      cout << "Got Publication: " << channel << " " << correlationId << ":"
           << streamId << ":" << sessionId << endl;
    });

    context.newSubscriptionHandler(
        [](const string &channel, int32_t streamId, int64_t correlationId) {
          cout << "Got Subscription: " << channel << " " << correlationId << ":"
               << streamId << endl;
        });

    context.availableImageHandler(
        [&shouldPoll, &subscriptionId, &loggerPublication, &serverSubscription,
         &settings](Image &image) {
          cout << "Available image correlationId=" << image.correlationId()
               << " sessionId=" << image.sessionId();
          cout << " at position=" << image.position() << " from "
               << image.sourceIdentity() << endl;

          if (image.subscriptionRegistrationId() != subscriptionId) return;

          shouldPoll = true;
        });

    context.unavailableImageHandler(
        [&shouldPoll, &subscriptionId](Image &image) {
          cout << "Unavailable image on correlationId=" << image.correlationId()
               << " sessionId=" << image.sessionId();
          cout << " at position=" << image.position() << " from "
               << image.sourceIdentity() << endl;

          if (image.subscriptionRegistrationId() != subscriptionId) return;

          shouldPoll = false;
        });

    Aeron aeron(context);

    publicationId =
        aeron.addPublication(settings.loggerChannel, settings.loggerStreamId);
    subscriptionId =
        aeron.addSubscription(settings.serverChannel, settings.serverStreamId);

    loggerPublication = aeron.findPublication(publicationId);
    while (!loggerPublication) {
      this_thread::yield();
      loggerPublication = aeron.findPublication(publicationId);
    }

    serverSubscription = aeron.findSubscription(subscriptionId);
    while (!serverSubscription) {
      this_thread::yield();
      serverSubscription = aeron.findSubscription(subscriptionId);
    }

    // setup engine
    LMDB::ErrorPtr e;
    auto Engine =
        EVENT_LOGGER::ENGINE::Engine(e, settings.dbPath, settings.logDBPath,
                                     settings.metaDBPath, settings.metaDBSize);

    while (running) {
      if (!shouldPoll) {
        this_thread::yield();

      } else {
        // This is our message handler.
        FragmentAssembler fragmentAssembler([&](AtomicBuffer &buffer,
                                                index_t offset, index_t length,
                                                Header &header) {
          messageHandler(Engine, loggerPublication, buffer, offset, length);
        });

        BusySpinIdleStrategy idleStrategy;
        while (shouldPoll && running &&
               serverSubscription->poll(fragmentAssembler.handler(),
                                        settings.fragmentCountLimit) <= 0) {
          idleStrategy.idle(0);
        }
      }
    }
  } catch (CommandOptionException &e) {
    cerr << "ERROR: " << e.what() << endl
         << endl;
    cerr << e.what();
    return -1;
  } catch (SourcedException &e) {
    cerr << "FAILED: " << e.what() << " : " << e.where() << endl;
    return -1;
  } catch (std::exception &e) {
    cerr << "FAILED: " << e.what() << " : " << endl;
    return -1;
  }

  return 0;
}
