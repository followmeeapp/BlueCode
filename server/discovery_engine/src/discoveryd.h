//
//  discoveryd.hpp
//  discovery_engine
//
//  Created by Erik van der Tier on 28/09/2016.
//
//

#ifndef discoveryd_h
#define discoveryd_h

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <exception>

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"
#include "aeron/util/CommandOptionParser.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"

#include "schema/settings.capnp.h"

#include "discovery_engine/engine/engine.h"
#include "clock/clock.h"

using namespace aeron;
using namespace DISCOVERY_ENGINE::ENGINE;

void sigIntHandler(int param);

template <typename ClockSourcePolicy>
void messageHandler(engine<ClockSourcePolicy> &Engine,
                    std::shared_ptr<aeron::Publication> enginePublication,
                    aeron::AtomicBuffer &buffer, index_t offset, index_t length,
                    std::atomic<bool> &running) {
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
      std::cout << "sent sync request: " << deviceID << std::endl;
      ResponseBody.setSyncRequest(request);
    });

    Engine.onSendDiscoverResponse([&ResponseBuilder, &ResponseBody](
        ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response) {
      std::cout << "Got discover response: " << deviceID << std::endl;
      ResponseBody.setDiscoverResponse(response);
    });

    switch (which) {
      case RPC::Kind::DISCOVER_REQUEST:
        std::cout << "Got discover request: " << deviceID << std::endl;
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
        std::cout << "Got sync request: " << deviceID << std::endl;
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
        std::cout << "Got discover ack request: " << deviceID << std::endl;
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
  } catch (std::exception &e) {
    didError = true;
    needTocloseConnection = true;
    ErrorStr = std::string(e.what());
  }

  if (didError) {
    std::cout << ErrorStr;
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

template <typename ClockSourcePolicy>
int runServer(DiscoverydSettings::Reader settings,
              xy::clock<ClockSourcePolicy> &clock, std::atomic<bool> &running) {
  try {
    ErrorPtr e;
    LMDBEnv EngineEnv =
        LMDBEnv(e, settings.getDbPath(), 16, settings.getDbSize());

    auto Engine = engine<ClockSourcePolicy>(e, EngineEnv, clock);

    std::cout << "Discovery Engine" << std::endl;
    std::cout << "Publishing Engine at " << settings.getEngineChannel().cStr()
              << " on Stream ID " << settings.getEngineStreamId() << std::endl;
    std::cout << "Subscribing Server at " << settings.getServerChannel().cStr()
              << " on Stream ID " << settings.getServerStreamId() << std::endl;

    aeron::Context context;
    int64_t publicationId;
    int64_t subscriptionId;
    std::atomic<bool> shouldPoll(false);

    std::shared_ptr<Publication> enginePublication = nullptr;
    std::shared_ptr<Subscription> serverSubscription = nullptr;

    if (settings.getDirPrefix() != "") {
      context.aeronDir(settings.getDirPrefix());
    }

    context.newPublicationHandler([](const std::string &channel,
                                     int32_t streamId, int32_t sessionId,
                                     int64_t correlationId) {
      std::cout << "Got Publication: " << channel << " " << correlationId << ":"
                << streamId << ":" << sessionId << std::endl;
    });

    context.newSubscriptionHandler([](const std::string &channel,
                                      int32_t streamId, int64_t correlationId) {
      std::cout << "Got Subscription: " << channel << " " << correlationId
                << ":" << streamId << std::endl;
    });

    context.availableImageHandler(
        [&shouldPoll, &subscriptionId, &Engine, &enginePublication,
         &serverSubscription, &settings](Image &image) {
          std::cout << "Available image correlationId=" << image.correlationId()
                    << " sessionId=" << image.sessionId();
          std::cout << " at position=" << image.position() << " from "
                    << image.sourceIdentity() << std::endl;

          if (image.subscriptionRegistrationId() != subscriptionId) return;

          shouldPoll = true;
        });

    context.unavailableImageHandler([&shouldPoll, &subscriptionId](
        Image &image) {
      std::cout << "Unavailable image on correlationId="
                << image.correlationId() << " sessionId=" << image.sessionId();
      std::cout << " at position=" << image.position() << " from "
                << image.sourceIdentity() << std::endl;

      if (image.subscriptionRegistrationId() != subscriptionId) return;

      shouldPoll = false;
    });

    Aeron aeron(context);

    publicationId = aeron.addPublication(settings.getEngineChannel(),
                                         settings.getEngineStreamId());
    subscriptionId = aeron.addSubscription(settings.getServerChannel(),
                                           settings.getServerStreamId());

    enginePublication = aeron.findPublication(publicationId);
    while (!enginePublication) {
      std::this_thread::yield();
      enginePublication = aeron.findPublication(publicationId);
    }

    serverSubscription = aeron.findSubscription(subscriptionId);
    while (!serverSubscription) {
      std::this_thread::yield();
      serverSubscription = aeron.findSubscription(subscriptionId);
    }

    while (running) {
      if (!shouldPoll) {
        std::this_thread::yield();

      } else {
        // This is our message handler.
        FragmentAssembler fragmentAssembler([&](AtomicBuffer &buffer,
                                                index_t offset, index_t length,
                                                Header &header) {
          messageHandler(Engine, enginePublication, buffer, offset, length,
                         running);
        });

        BusySpinIdleStrategy idleStrategy;
        while (shouldPoll && running &&
               serverSubscription->poll(fragmentAssembler.handler(),
                                        settings.getFragmentCountLimit()) <=
                   0) {
          idleStrategy.idle(0);
        }
      }
    }

  } catch (SourcedException &e) {
    std::cerr << "FAILED: " << e.what() << " : " << e.where() << std::endl;
    return -1;

  } catch (std::exception &e) {
    std::cerr << "FAILED: " << e.what() << " : " << std::endl;
    return -1;
  }
  return 0;
}

#endif /* discoveryd_hpp */
