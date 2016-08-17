//
//  main.cpp
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <cstdint>
#include <cstdio>
#include <signal.h>
#include <thread>
#include <array>

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/util/CommandOptionParser.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"

#include "../config.h"

using namespace std;
using namespace aeron::util;
using namespace aeron;

std::atomic<bool> running(true);
static int deviceId = 1;

void sigIntHandler(int param) { running = false; }

static const char optHelp = 'h';

// Aeron options
static const char optPrefix = 'p';
static const char optServerChannel = 'c';
static const char optEngineChannel = 'C';
static const char optServerStreamId = 's';
static const char optEngineStreamId = 'S';
static const char optFrags = 'f';

struct Settings {
  string dirPrefix = "/Users/erich/Desktop/aeron";
  string serverChannel = DISCOVERY_ENGINE::CONFIG::DEFAULT_SERVER_CHANNEL;
  string engineChannel = DISCOVERY_ENGINE::CONFIG::DEFAULT_ENGINE_CHANNEL;
  int32_t serverStreamId = DISCOVERY_ENGINE::CONFIG::DEFAULT_SERVER_STREAM_ID;
  int32_t engineStreamId = DISCOVERY_ENGINE::CONFIG::DEFAULT_ENGINE_STREAM_ID;
  int32_t fragmentCountLimit =
      DISCOVERY_ENGINE::CONFIG::DEFAULT_FRAGMENT_COUNT_LIMIT;
};

auto parseCmdLine(CommandOptionParser& cp, int argc, char** argv) -> Settings {
  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.dirPrefix = cp.getOption(optPrefix).getParam(0, s.dirPrefix);
  s.serverChannel = cp.getOption(optServerChannel).getParam(0, s.serverChannel);
  s.engineChannel = cp.getOption(optEngineChannel).getParam(0, s.engineChannel);
  s.serverStreamId = cp.getOption(optServerStreamId)
                         .getParamAsInt(0, 1, INT32_MAX, s.serverStreamId);
  s.engineStreamId = cp.getOption(optEngineStreamId)
                         .getParamAsInt(0, 1, INT32_MAX, s.engineStreamId);
  s.fragmentCountLimit = cp.getOption(optFrags).getParamAsInt(
      0, 1, INT32_MAX, s.fragmentCountLimit);

  return s;
}

void messageHandler(std::shared_ptr<Publication> serverPublication,
                    AtomicBuffer& buffer, index_t offset, index_t length) {
  uint8_t* data = buffer.buffer() + offset;
  kj::ArrayPtr<const capnp::word> view((const capnp::word*)data, length / 8);
  auto reader = capnp::FlatArrayMessageReader(view);

  auto request = reader.getRoot<RPC>();
  int64_t deviceID = request.getDevice();
  cout << "got response for device: " << deviceID << endl;

  auto requestKind = request.getKind();
  auto which = requestKind.which();
  switch (which) {
    case RPC::Kind::DISCOVER_RESPONSE:
      cout << "got discover response" << endl;
      break;

    case RPC::Kind::SYNC_REQUEST:
      cout << "got sync request" << endl;
      break;

    default:
      break;
  }
}

int main(int argc, char** argv) {
  CommandOptionParser cp;
  cp.addOption(CommandOption(optHelp, 0, 0,
                             "                Displays help information."));

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
    Settings settings = parseCmdLine(cp, argc, argv);

    cout << "Tester" << endl;
    cout << "Publishing Server at " << settings.serverChannel
         << " on Stream ID " << settings.serverStreamId << endl;
    cout << "Subscribing Engine at " << settings.engineChannel
         << " on Stream ID " << settings.engineStreamId << endl;

    aeron::Context context;
    int64_t publicationId;
    int64_t subscriptionId;
    std::atomic<bool> shouldPoll(false);

    shared_ptr<Publication> serverPublication = nullptr;
    shared_ptr<Subscription> engineSubscription = nullptr;

    if (settings.dirPrefix != "") {
      context.aeronDir(settings.dirPrefix);
    }

    context.newPublicationHandler([](const string& channel, int32_t streamId,
                                     int32_t sessionId, int64_t correlationId) {
      cout << "Publication: " << channel << " " << correlationId << ":"
           << streamId << ":" << sessionId << endl;
    });

    context.newSubscriptionHandler(
        [](const string& channel, int32_t streamId, int64_t correlationId) {
          cout << "Subscription: " << channel << " " << correlationId << ":"
               << streamId << endl;
        });

    context.availableImageHandler(
        [&shouldPoll, &subscriptionId, &serverPublication, &engineSubscription,
         &settings](Image& image) {
          cout << "Available image correlationId=" << image.correlationId()
               << " sessionId=" << image.sessionId();
          cout << " at position=" << image.position() << " from "
               << image.sourceIdentity() << endl;

          if (image.subscriptionRegistrationId() != subscriptionId) return;

          shouldPoll = true;

          // Send an initial message to the discovery engine.
          capnp::MallocMessageBuilder builder;
          auto request = builder.initRoot<RPC>();
          request.setDevice(deviceId++);
          auto discover = request.initKind().initDiscoverRequest();
          auto discInfo = discover.initDiscoverInfo();
          discInfo.setType(DiscoverInfo::Type::DISCOVER);
          kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
          kj::ArrayPtr<kj::byte> bytes = words.asBytes();

          concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
          do {
            // wait to publish message successfully
          } while (running &&
                   serverPublication->offer(srcBuffer, 0, bytes.size()) < 0L);

          cout << "Published discover request" << endl;

          // This is our message handler.
          FragmentAssembler fragmentAssembler(
              [&](AtomicBuffer& buffer, index_t offset, index_t length,
                  Header& header) {
                messageHandler(serverPublication, buffer, offset, length);
              });

          // Poll for messages from discovery engine.
          BusySpinIdleStrategy idleStrategy;
          while (shouldPoll && running &&
                 engineSubscription->poll(fragmentAssembler.handler(),
                                          settings.fragmentCountLimit) <= 0) {
            idleStrategy.idle(0);
          }
        });

    context.unavailableImageHandler(
        [&shouldPoll, &subscriptionId](Image& image) {
          cout << "Unavailable image on correlationId=" << image.correlationId()
               << " sessionId=" << image.sessionId();
          cout << " at position=" << image.position() << " from "
               << image.sourceIdentity() << endl;

          if (image.subscriptionRegistrationId() != subscriptionId) return;

          shouldPoll = false;
        });

    Aeron aeron(context);

    publicationId =
        aeron.addPublication(settings.serverChannel, settings.serverStreamId);
    subscriptionId =
        aeron.addSubscription(settings.engineChannel, settings.engineStreamId);

    serverPublication = aeron.findPublication(publicationId);
    while (!serverPublication) {
      this_thread::yield();
      serverPublication = aeron.findPublication(publicationId);
    }

    engineSubscription = aeron.findSubscription(subscriptionId);
    while (!engineSubscription) {
      this_thread::yield();
      engineSubscription = aeron.findSubscription(subscriptionId);
    }

    // Wait for an image for the server subscription to become available.
    while (running) this_thread::yield();

  } catch (CommandOptionException& e) {
    cerr << "ERROR: " << e.what() << endl
         << endl;
    cp.displayOptionsHelp(cerr);
    return -1;

  } catch (SourcedException& e) {
    cerr << "FAILED: " << e.what() << " : " << e.where() << endl;
    return -1;

  } catch (std::exception& e) {
    cerr << "FAILED: " << e.what() << " : " << endl;
    return -1;
  }

  return 0;
}
