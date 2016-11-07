//
//  //discovery_graph/graph/graph_test.cpp
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//

#include <gtest/gtest.h>
#include <stdlib.h>
#include <chrono>
#include "event_logger/engine/engine.h"
#include "event_logger/engine/generic.h"
using namespace EVENT_LOGGER::ENGINE;

namespace {

const auto SecondDuration = 1000;
const auto MinuteDuration = SecondDuration * 60;
const auto HourDuration = MinuteDuration * 60;

using std::string;
using namespace LMDB;

// The fixture for testing class Foo.
class engine_test : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  engine_test() {
    // You can do set-up work for each test here.
  }

  virtual ~engine_test() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
    string logCommand = string("rm ") + dbPath + string("/log.tbd");
    string metaCommand = string("rm ") + dbPath + string("/meta/*");
    system(logCommand.c_str());
    system(metaCommand.c_str());
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  string dbPath = "/Users/erik/Documents/Suiron/Data/event_log";
  string logDBPath = dbPath + "/log";
  string metaDBPath = dbPath + "/meta";
  size_t metaDBSize = 4096;
};

//________________________________________________________________________________
//
// NewDevice
//________________________________________________________________________________

TEST_F(engine_test, NewDevice) {
  ErrorPtr e;

  // set CurrentTime as a time baseline for interactions in this test

  auto Engine = EVENT_LOGGER::ENGINE::Engine(e, dbPath, logDBPath, metaDBPath,
                                             metaDBSize);

  capnp::MallocMessageBuilder traceBuilder;
  Trace::Builder trace = traceBuilder.initRoot<Trace>();
  trace.setRequestId(1);
  trace.setTimestamp(0);

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

  Engine.logTrail(e, trace);
  ASSERT_FALSE(e);
  Engine.logTrail(e, trace);
  ASSERT_TRUE(e);
  LOG(e.message())

}  // NewDevice

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
