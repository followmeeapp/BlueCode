//
//  main.cpp
//  Follow
//
// Copyright © 2016 Follow-Mee, Incorporated. All rights reserved.
//

#include <cstdint>
#include <cstdio>
#include <gtest/gtest.h>

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "schema/rpc.capnp.h"
#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/client.hpp"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;
using namespace std;
using namespace rapidjson;

// The fixture for testing class Foo.
class dashboard_server_test : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  dashboard_server_test() {
    // You can do set-up work for each test here.
  }

  virtual ~dashboard_server_test() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  std::string uri = "ws://localhost:3010";
  client c;
};

//__________________________________________________________________________________
//
// NewDevice
//__________________________________________________________________________________

TEST_F(dashboard_server_test, NewDevice) {
  auto onOpen = [](client* c, websocketpp::connection_hdl hdl) {
    // now it is safe to use the connection
    std::cout << "connection ready" << std::endl;

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    writer.StartObject();
    writer.Key("request");
    writer.String("get-trail");
    writer.Key("request-ids");
    writer.StartArray();
    writer.Uint(1);
    writer.Uint(2);
    writer.EndArray();
    writer.EndObject();
    std::cout << "sending json: " << s.GetString() << "\n";
    c->send(hdl, s.GetString(), s.GetSize(), websocketpp::frame::opcode::TEXT);
  };

  auto onMessage = [](client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << msg->get_payload() << std::endl;
  };

  try {
    // Set logging to be pretty verbose (everything except message payloads)
    c.set_access_channels(websocketpp::log::alevel::none);
    c.clear_access_channels(websocketpp::log::alevel::none);

    // Initialize ASIO
    c.init_asio();

    c.set_open_handler(bind(onOpen, &c, ::_1));
    // Register our message handler
    c.set_message_handler(bind(onMessage, &c, ::_1,::_2));

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    if (ec) {
      std::cout << "could not create connection because: " << ec.message()
                << std::endl;
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
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
