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
#include <iostream>

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/util/CommandOptionParser.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/trace.capnp.h"
#include "schema/join.capnp.h"

#include "../config.h"

#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using namespace std;
using namespace aeron::util;
using namespace aeron;
std::atomic<bool> running(true);

void sigIntHandler(int param) { running = false; }

static const char optHelp = 'h';

// Aeron options
static const char optPrefix = 'p';
static const char optServerChannel = 'c';
static const char optLoggerChannel = 'C';
static const char optServerStreamId = 's';
static const char optLoggerStreamId = 'S';
static const char optFrags = 'f';

struct Settings {
  string dirPrefix = "/Users/erik/Documents/Suiron/Data/aeron";
  string serverChannel = EVENT_LOGGER::CONFIG::DEFAULT_SERVER_CHANNEL;
  string loggerChannel = EVENT_LOGGER::CONFIG::DEFAULT_LOGGER_CHANNEL;
  int32_t serverStreamId = EVENT_LOGGER::CONFIG::DEFAULT_SERVER_STREAM_ID;
  int32_t loggerStreamId = EVENT_LOGGER::CONFIG::DEFAULT_LOGGER_STREAM_ID;
  int32_t fragmentCountLimit =
      EVENT_LOGGER::CONFIG::DEFAULT_FRAGMENT_COUNT_LIMIT;
};

auto parseCmdLine(int argc, char** argv) -> Settings {
  CommandOptionParser cp;
  cp.addOption(CommandOption(optHelp, 0, 0,
                             "                Displays help information."));

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

void messageHandler(std::shared_ptr<Publication> serverPublication,
                    AtomicBuffer& buffer, index_t offset, index_t length) {}

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

void on_open(client* c, websocketpp::connection_hdl hdl) {
  // now it is safe to use the connection
  std::cout << "connection ready" << std::endl;

  capnp::MallocMessageBuilder builder;
  auto Join = builder.initRoot<JoinRequest>();
  Join.setTelephone("blabla");
  kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
  kj::ArrayPtr<kj::byte> bytes = words.asBytes();

  c->send(hdl, (char*)(bytes.begin()), bytes.size(),
          websocketpp::frame::opcode::BINARY);
  c->send(hdl, (char*)(bytes.begin()), bytes.size(),
          websocketpp::frame::opcode::BINARY);
  c->send(hdl, (char*)(bytes.begin()), bytes.size(),
          websocketpp::frame::opcode::BINARY);
  c->send(hdl, (char*)(bytes.begin()), bytes.size(),
          websocketpp::frame::opcode::BINARY);
  c->send(hdl, (char*)(bytes.begin()), bytes.size(),
          websocketpp::frame::opcode::BINARY);
}

int main(int argc, char** argv) {
  std::string uri = "ws://localhost:3000";
  client c;

  try {
    // Set logging to be pretty verbose (everything except message payloads)
    c.set_access_channels(websocketpp::log::alevel::none);
    c.clear_access_channels(websocketpp::log::alevel::none);

    // Initialize ASIO
    c.init_asio();

    c.set_open_handler(bind(&on_open, &c, ::_1));
    // Register our message handler
    // c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    if (ec) {
      std::cout << "could not create connection because: " << ec.message()
                << std::endl;
      return 0;
    }

    // Note that connect here only requests a connection. No network messages
    // are
    // exchanged until the event loop starts running in the next line.
    c.connect(con);

    // Start the ASIO io_service run loop
    // this will cause a single connection to be made to the server. c.run()
    // will exit when this connection is closed.
    c.run();
  } catch (websocketpp::exception const& e) {
    std::cout << e.what() << std::endl;
  }
  /*
      signal(SIGINT, sigIntHandler);

      try {
        Settings settings = parseCmdLine(argc, argv);

        cout << "Tester" << endl;
        cout << "Publishing Server at " << settings.serverChannel
             << " on Stream ID " << settings.serverStreamId << endl;
        cout << "Subscribing Engine at " << settings.loggerChannel
        << " on Stream ID " << settings.loggerStreamId << endl;

        aeron::Context context;
        int64_t publicationId;
        int64_t subscriptionId;
        std::atomic<bool> shouldPoll(false);

        shared_ptr<Publication> serverPublication = nullptr;
        shared_ptr<Subscription> loggerSubscription = nullptr;

        if (settings.dirPrefix != "") {
          context.aeronDir(settings.dirPrefix);
        }

        context.newPublicationHandler([](const string& channel, int32_t
     streamId,
                                         int32_t sessionId, int64_t
     correlationId)
      {
          cout << "Publication: " << channel << " " << correlationId << ":"
               << streamId << ":" << sessionId << endl;
        });

        context.newSubscriptionHandler(
            [](const string& channel, int32_t streamId, int64_t correlationId) {
              cout << "Subscription: " << channel << " " << correlationId << ":"
                   << streamId << endl;
            });

        context.availableImageHandler(
            [&shouldPoll, &subscriptionId, &serverPublication,
      &loggerSubscription,
             &settings](Image& image) {
              cout << "Available image correlationId=" << image.correlationId()
                   << " sessionId=" << image.sessionId();
              cout << " at position=" << image.position() << " from "
                   << image.sourceIdentity() << endl;

              if (image.subscriptionRegistrationId() != subscriptionId) return;

              shouldPoll = true;

              capnp::MallocMessageBuilder traceBuilder;
              Trace::Builder trace = traceBuilder.initRoot<Trace>();
              trace.setRequestId(1);

              auto events = trace.initEvents(3);
              events[0].setTimestamp(0);
              events[0].setType(Trace::Event::Type::BEGIN);
              events[0].setFunction(Trace::Event::Function::READ_REQUEST);
              events[1].setTimestamp(1);
              events[1].setType(Trace::Event::Type::END);
              events[1].setFunction(Trace::Event::Function::READ_REQUEST);
              events[2].setTimestamp(2);
              events[2].setType(Trace::Event::Type::THROW);
              events[2].setFunction(Trace::Event::Function::READ_REQUEST);

              // Publish trace
              kj::Array<capnp::word> words =
     capnp::messageToFlatArray(traceBuilder);
              kj::ArrayPtr<kj::byte> bytes = words.asBytes();
              concurrent::AtomicBuffer msgBuffer(bytes.begin(), bytes.size());
              do {
                // wait to publish message successfully
              } while (running &&
                       serverPublication->offer(msgBuffer, 0,
     sizeof(req_event_t))
      <
                           0L);

              cout << "Published event" << endl;

              // This is our message handler.
              FragmentAssembler fragmentAssembler(
                  [&](AtomicBuffer& buffer, index_t offset, index_t length,
                      Header& header) {
                    messageHandler(serverPublication, buffer, offset, length);
                  });

              // Poll for messages from discovery engine.
              BusySpinIdleStrategy idleStrategy;
              while (shouldPoll && running &&
                     loggerSubscription->poll(fragmentAssembler.handler(),
                                              settings.fragmentCountLimit) <= 0)
     {
                idleStrategy.idle(0);
              }
            });

        context.unavailableImageHandler(
            [&shouldPoll, &subscriptionId](Image& image) {
              cout << "Unavailable image on correlationId=" <<
      image.correlationId()
                   << " sessionId=" << image.sessionId();
              cout << " at position=" << image.position() << " from "
                   << image.sourceIdentity() << endl;

              if (image.subscriptionRegistrationId() != subscriptionId) return;

              shouldPoll = false;
            });

        Aeron aeron(context);

        publicationId =
            aeron.addPublication(settings.serverChannel,
     settings.serverStreamId);
        subscriptionId =
            aeron.addSubscription(settings.loggerChannel,
      settings.loggerStreamId);

        serverPublication = aeron.findPublication(publicationId);
        while (!serverPublication) {
          this_thread::yield();
          serverPublication = aeron.findPublication(publicationId);
        }

        loggerSubscription = aeron.findSubscription(subscriptionId);
        while (!loggerSubscription) {
          this_thread::yield();
          loggerSubscription = aeron.findSubscription(subscriptionId);
        }

        // Wait for an image for the server subscription to become available.
        while (running) this_thread::yield();

      } catch (CommandOptionException& e) {
        cerr << "ERROR: " << e.what() << endl
             << endl;
        cerr << e.what();
        return -1;

      } catch (SourcedException& e) {
        cerr << "FAILED: " << e.what() << " : " << e.where() << endl;
        return -1;

      } catch (std::exception& e) {
        cerr << "FAILED: " << e.what() << " : " << endl;
        return -1;
      } */
  return 0;
}
