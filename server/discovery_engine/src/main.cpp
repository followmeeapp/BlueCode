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

#include "discovery_engine/cpp_lmdb/lmdb_env.h"  // TODO: move error stuff to separate file
#include "discovery_engine/engine/engine.h"

#include "schema/rpc.capnp.h"

#include "config.h"

using namespace std;
using namespace std::chrono;
using namespace aeron::util;
using namespace aeron;
using namespace DISCOVERY_ENGINE::ENGINE;
using namespace LMDB;

auto make_reader(MDB_val data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 &&
         "expected data.mv_size to be a multiple of 8");

  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data,
                                       data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}

std::atomic<bool> running(true);

void sigIntHandler(int param) { running = false; }

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

struct Settings {
  string dbPath = "/Users/erich/Desktop/discoveryd";
  int32_t dbSize = DISCOVERY_ENGINE::CONFIG::DEFAULT_DB_SIZE;

  string dirPrefix = "/Users/erich/Desktop/aeron";
  string serverChannel = DISCOVERY_ENGINE::CONFIG::DEFAULT_SERVER_CHANNEL;
  string engineChannel = DISCOVERY_ENGINE::CONFIG::DEFAULT_ENGINE_CHANNEL;
  int32_t serverStreamId = DISCOVERY_ENGINE::CONFIG::DEFAULT_SERVER_STREAM_ID;
  int32_t engineStreamId = DISCOVERY_ENGINE::CONFIG::DEFAULT_ENGINE_STREAM_ID;
  int32_t fragmentCountLimit =
      DISCOVERY_ENGINE::CONFIG::DEFAULT_FRAGMENT_COUNT_LIMIT;
};

auto parseCmdLine(CommandOptionParser &cp, int argc, char **argv) -> Settings {
  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.dbPath = cp.getOption(optDbPath).getParam(0, s.dbPath);
  s.dbSize = cp.getOption(optDbSize).getParamAsInt(0, 1, INT32_MAX, s.dbSize);

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

void messageHandler(engine &Engine,
                    std::shared_ptr<Publication> enginePublication,
                    AtomicBuffer &buffer, index_t offset, index_t length) {
  bool didError = false;
  bool needTocloseConnection = false;
  std::string ErrorStr = "";
  capnp::MallocMessageBuilder ResponseBuilder;
  // We *always* send a response.
  auto Response = ResponseBuilder.initRoot<RPC>();
  auto ResponseBody = Response.getKind();
  try {
    uint8_t *data = buffer.buffer() + offset;
    kj::ArrayPtr<const capnp::word> view((const capnp::word *)data, length / 8);
    auto reader = capnp::FlatArrayMessageReader(view);

    auto request = reader.getRoot<RPC>();
    int64_t deviceID = request.getDevice();
    Response.setDevice(deviceID);
    auto requestKind = request.getKind();
    auto which = requestKind.which();

    ErrorPtr e;

    // setup response handlers

    Engine.onSendSyncRequest([&ResponseBuilder, &ResponseBody](
        ErrorPtr &e, int64_t deviceID, SyncRequest::Reader request) {
      cout << "sent sync request: " << deviceID << endl;
      ResponseBody.setSyncRequest(request);
    });

    Engine.onSendDiscoverResponse([&ResponseBuilder, &ResponseBody](
        ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response) {
      cout << "Got discover response: " << deviceID << endl;
      ResponseBody.setDiscoverResponse(response);
    });

    switch (which) {
      case RPC::Kind::DISCOVER_REQUEST:
        cout << "Got discover request: " << deviceID << endl;
        // call request handler within a transaction

        Engine.beginTransaction(e, [&Engine, &request, &requestKind](
                                       ErrorPtr &e, Transaction &txn) {
          Engine.handleDiscoverRequest(e, request.getDevice(),
                                       requestKind.getDiscoverRequest(), txn);
          ON_ERROR_RETURN(e);
          Engine.commitTransaction(e, txn);
          ON_ERROR_RETURN(e);
        });
        if (e) {
          didError = true;
          needTocloseConnection = true;
          ErrorStr = e.message();
        }
        break;
      case RPC::Kind::SYNC_RESPONSE:
        cout << "Got sync request: " << deviceID << endl;
        Engine.beginTransaction(e, [&Engine, &request, &requestKind](
                                       ErrorPtr &e, Transaction &txn) {
          Engine.handleSyncResponse(e, request.getDevice(),
                                    requestKind.getSyncResponse(), txn);
          ON_ERROR_RETURN(e);
          Engine.commitTransaction(e, txn);
          ON_ERROR_RETURN(e);
        });
        if (e) {
          didError = true;
          needTocloseConnection = true;
          ErrorStr = e.message();
        }
        break;
      case RPC::Kind::DISCOVER_ACK_RESPONSE:
        cout << "Got discover ack request: " << deviceID << endl;
        Engine.beginTransaction(e, [&Engine, &request, &requestKind](
                                       ErrorPtr &e, Transaction &txn) {
          Engine.handleDiscoverAckResponse(e, request.getDevice(),
                                           requestKind.getDiscoverAckResponse(),
                                           txn);
          ON_ERROR_RETURN(e);
          Engine.commitTransaction(e, txn);
          ON_ERROR_RETURN(e);
        });
        if (e) {
          didError = true;
          needTocloseConnection = true;
          ErrorStr = e.message();
        }
        break;
      default:
        break;
    }  // switch (which)
    if (which == RPC::Kind::DISCOVER_ACK_RESPONSE)
      return;  // no response should be send
  } catch (kj::Exception &e) {
    didError = true;
    needTocloseConnection = true;
    ErrorStr = e.getDescription();
  } catch (exception &e) {
    didError = true;
    needTocloseConnection = true;
    ErrorStr = std::string(e.what());
  }

  if (didError) {
    cout << ErrorStr;
    auto error = ResponseBody.initErrorResponse();
    error.setMessage(ErrorStr);
  }

  kj::Array<capnp::word> words = capnp::messageToFlatArray(ResponseBuilder);
  kj::ArrayPtr<kj::byte> bytes = words.asBytes();

  // Send a message to the server.
  concurrent::AtomicBuffer srcBuffer(bytes.begin(), bytes.size());
  do {
    // wait to publish message successfully
  } while (running &&
           enginePublication->offer(srcBuffer, 0, bytes.size()) < 0L);
}

auto main(int argc, char **argv) -> int {
  assert(sizeof(unsigned long long) == sizeof(void *) &&
         "Expect sizeof(unsigned long long) == sizeof(void *)");

  CommandOptionParser cp;
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
    Settings settings = parseCmdLine(cp, argc, argv);

    ErrorPtr e;
    LMDBEnv EngineEnv = LMDBEnv(e, settings.dbPath, 16, settings.dbSize);
    auto Engine = engine(e, EngineEnv);

    cout << "Discovery Engine" << endl;
    cout << "Publishing Engine at " << settings.engineChannel
         << " on Stream ID " << settings.engineStreamId << endl;
    cout << "Subscribing Server at " << settings.serverChannel
         << " on Stream ID " << settings.serverStreamId << endl;

    aeron::Context context;
    int64_t publicationId;
    int64_t subscriptionId;
    std::atomic<bool> shouldPoll(false);

    std::shared_ptr<Publication> enginePublication = nullptr;
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
        [&shouldPoll, &subscriptionId, &Engine, &enginePublication,
         &serverSubscription, &settings](Image &image) {
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
        aeron.addPublication(settings.engineChannel, settings.engineStreamId);
    subscriptionId =
        aeron.addSubscription(settings.serverChannel, settings.serverStreamId);

    enginePublication = aeron.findPublication(publicationId);
    while (!enginePublication) {
      this_thread::yield();
      enginePublication = aeron.findPublication(publicationId);
    }

    serverSubscription = aeron.findSubscription(subscriptionId);
    while (!serverSubscription) {
      this_thread::yield();
      serverSubscription = aeron.findSubscription(subscriptionId);
    }

    while (running) {
      if (!shouldPoll) {
        this_thread::yield();

      } else {
        // This is our message handler.
        FragmentAssembler fragmentAssembler([&](AtomicBuffer &buffer,
                                                index_t offset, index_t length,
                                                Header &header) {
          messageHandler(Engine, enginePublication, buffer, offset, length);
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
    cp.displayOptionsHelp(cerr);
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
