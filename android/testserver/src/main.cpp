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

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"

#include "schema/card_info.capnp.h"
#include "uWebSockets/uWS.h"
#include "config.h"
#include "generic.h"
#include "string.h"

using namespace std;
using namespace std::chrono;


int connections = 0;

/*auto inline make_reader(DBVal data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 &&
         "expected data.mv_size to be a multiple of 8");
  
  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data,
                                       data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}*/


int main() {
  assert(sizeof(unsigned long long) == sizeof(void *) &&
         "Expect sizeof(unsigned long long) == sizeof(void *)");

  capnp::MallocMessageBuilder messageBuilder;
  
  try {

    uWS::Server server(3000);

    server.onConnection([](uWS::ServerSocket socket) {
      cout << "[Connection] clients: " << ++connections << endl;
    });

    server.onMessage([](uWS::ServerSocket socket, const char *msg,
                                  size_t len, uWS::OpCode opCode) {

      bool didError = false;
      bool needTocloseConnection = false;
      std::string ErrorStr = "";

      std::cout << "size: " << len;
      kj::ArrayPtr<const capnp::word> view((const capnp::word *)msg,
                                           len);
      
      auto card = capnp::FlatArrayMessageReader(view).getRoot<CardInfo>();
      std::cout << "data: " << card.getId() << card.getTimestamp() << std::endl;
      std::string resultString = "card received id:" + to_string(card.getId()) + " timeStamp: " + to_string(card.getTimestamp());
      socket.send((char*)resultString.c_str(), resultString.size(), uWS::OpCode::TEXT);
      
      if (didError && needTocloseConnection) {
        return 1;
      }
      return 0;
    });

    server.onDisconnection(
        [](uWS::ServerSocket socket, int code, char *message, size_t length) {
          cout << "[Disconnection] clients: " << --connections << endl;
          socket.setData(nullptr);
        });

    server.run();

  } catch (std::exception &e) {
    cerr << "FAILED: " << e.what() << " : " << endl;
    return -1;
  }
  return 0;
}
