//
//  nop.cpp
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <iostream>
#include <string>
#include <thread>

#include <cassert>

using namespace std;

#include "uWebSockets/uWS.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "rpc/rpc.capnp.h"

int connections = 0;

int main()
{
  assert(sizeof(unsigned long long) == sizeof(void *) && "Expect sizeof(unsigned long long) == sizeof(void *)");

  try {
    uWS::Server server(3003);

    server.onConnection([](uWS::Socket socket) {
      cout << "[Connection] clients: " << ++connections << endl;
    });

    server.onMessage([](uWS::Socket socket, const char *msg, size_t len, uWS::OpCode opCode) {
      try {
        kj::ArrayPtr<const capnp::word> view((const capnp::word *)msg, len / 8);
        auto reader = capnp::FlatArrayMessageReader(view);
        RPC::Reader rpc = reader.getRoot<RPC>();
        int64_t device = rpc.getDevice();
        auto kind = rpc.getKind();
        auto which = kind.which();

        if (which == RPC::Kind::DISCOVER_REQUEST) {
          cout << "Got discover request: " << device << endl;
          DiscoverRequest::Reader discoverRequest = kind.getDiscoverRequest();

          // Do something with discoverRequest in a real server. Since this in a no-op server,
          // we just construct a default discover response and send it back.

          capnp::MallocMessageBuilder builder;
          RPC::Builder response = builder.initRoot<RPC>();
          response.setDevice(device);
          auto responseKind = response.getKind();
          auto discoverResponse = responseKind.initDiscoverResponse();

          // We'll leave everything else set to their default values.

          kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
          kj::ArrayPtr<kj::byte> bytes = words.asBytes();

          socket.send((char *)(bytes.begin()), bytes.size(), uWS::OpCode::BINARY);

        } else if (which == RPC::Kind::SYNC_RESPONSE) {
          cout << "Got sync response (unexpectedly): " << device << endl;
          SyncResponse::Reader syncResponse = kind.getSyncResponse();

          // Do something with syncResponse in a real server. Since this in a no-op server,
          // we never send any SyncRequests, so do not expect to get sent a SyncResponse.

        } else if (which == RPC::Kind::DISCOVER_ACK_RESPONSE) {
          cout << "Got discover ACK response: " << device << endl;
          DiscoverAckResponse::Reader discoverAckResponse = kind.getDiscoverAckResponse();

          // Do something with discoverAckResponse in a real server. Since this in a no-op server,
          // we just ignore it.

        } else {
          cout << "Unknown RPC type: " << int(which) << endl;
        }

      } catch (...) {
        cout << "Unknown error handling message" << endl;
      }
    });

    server.onDisconnection([](uWS::Socket socket, int code, char *message, size_t length) {
      cout << "[Disconnection] clients: " << --connections << endl;
    });

    server.run(); // noreturn

  } catch (...) {
    cout << "ERR_LISTEN" << endl;
  }

  return 0;
}
