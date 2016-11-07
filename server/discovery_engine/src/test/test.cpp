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
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <spawn.h>
#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/util/CommandOptionParser.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"
#include "schema/test_processes.capnp.h"

#include "../config.h"
#include "../discoveryd.h"
#include "clock/clock.h"
#include "test_process_runner/test_process_runner.h"

using namespace std;
using namespace aeron::util;
using namespace aeron;

std::atomic<bool> running(true);
static int deviceId = 1;

void sigIntHandler(int param) {
  std::cout << "sigkill";
  running = false;
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

int main(int argc, char** argv, char* env[]) {
  signal(SIGKILL, sigIntHandler);
  capnp::MallocMessageBuilder SettingsBuilder;

  auto SettingsRoot = SettingsBuilder.initRoot<TestProcesses>();
  auto ServerList = SettingsRoot.initServers(1);

  ServerList[0].setClockRegion("ClockRegion3");
  auto DiscSetting = ServerList[0].initSettings().initDiscoveryd();

  using namespace DISCOVERY_ENGINE::CONFIG;

  DiscSetting.setDbPath(DEFAULT_DB_PATH);
  DiscSetting.setDbSize(DEFAULT_DB_SIZE);
  DiscSetting.setDirPrefix(DEFAULT_DIR_PREFIX);
  DiscSetting.setServerChannel(DEFAULT_SERVER_CHANNEL);
  DiscSetting.setEngineChannel(DEFAULT_ENGINE_CHANNEL);
  DiscSetting.setServerStreamId(DEFAULT_SERVER_STREAM_ID);
  DiscSetting.setEngineStreamId(DEFAULT_ENGINE_STREAM_ID);
  DiscSetting.setFragmentCountLimit(DEFAULT_FRAGMENT_COUNT_LIMIT);

  auto ServerRunner = ServerProcessRunner(env);
  ServerRunner.setDiscoveryDRunFunction(::runServer<xy::SharedMemorySource>);
  auto ret = ServerRunner.runServerProcesses(SettingsRoot);

  if (ret == -1) return 1;
  try {
    cout << "Tester" << endl;
    cout << "Publishing Server at " << DiscSetting.getServerChannel().cStr()
         << " on Stream ID " << DiscSetting.getServerStreamId() << endl;
    cout << "Subscribing Engine at " << DiscSetting.getEngineChannel().cStr()
         << " on Stream ID " << DiscSetting.getEngineStreamId() << endl;

    aeron::Context context;
    int64_t publicationId;
    int64_t subscriptionId;
    std::atomic<bool> shouldPoll(false);

    std::shared_ptr<Publication> serverPublication = nullptr;
    std::shared_ptr<Subscription> engineSubscription = nullptr;

    if (DiscSetting.getDirPrefix().asString() != "") {
      context.aeronDir(DiscSetting.getDirPrefix().cStr());
    }

    ServerRunner.getClock("ClockRegion3").setTime(100);

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
         &DiscSetting](Image& image) {
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
                 engineSubscription->poll(
                     fragmentAssembler.handler(),
                     DiscSetting.getFragmentCountLimit()) <= 0) {
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
        aeron.addPublication(DiscSetting.getServerChannel().asString(),
                             DiscSetting.getServerStreamId());
    subscriptionId =
        aeron.addSubscription(DiscSetting.getEngineChannel().asString(),
                              DiscSetting.getEngineStreamId());

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

  } catch (SourcedException& e) {
    cerr << "FAILED: " << e.what() << " : " << e.where() << endl;
    return -1;

  } catch (std::exception& e) {
    cerr << "FAILED: " << e.what() << " : " << endl;
    return -1;
  }

  return 0;
}
