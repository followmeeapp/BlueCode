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
#include "dashboard_server/engine/engine.h"
#include "dashboard_server/engine/generic.h"
#include "rapidjson/prettywriter.h"
using namespace DASHBOARD_SERVER::ENGINE;

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

//__________________________________________________________________________________
//
// handleGetRequestIDs
//__________________________________________________________________________________

TEST_F(engine_test, handleGetRequestIDs) {
  ErrorPtr e;

  // set CurrentTime as a time baseline for interactions in this test
  auto CurrentTime = now();

  auto Engine = DASHBOARD_SERVER::ENGINE::Engine(e, dbPath, logDBPath,
                                                 metaDBPath, metaDBSize);

  Document Request = Document(Type::kObjectType);
  auto &allocator = Request.GetAllocator();

  Request.AddMember("request", Value("get-request-ids"), allocator);

  auto Response = Engine.handleGetRequestIDs(e, Request);

  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  Response.Accept(writer);
  LOG("handleGetRequestIDs Response JSON: " << sb.GetString())

}  // handleGetRequestIDs

//__________________________________________________________________________________
//
// GetTrailRequest
//__________________________________________________________________________________

TEST_F(engine_test, GetTrailRequest) {
  ErrorPtr e;

  // set CurrentTime as a time baseline for interactions in this test
  auto CurrentTime = now();

  auto Engine = DASHBOARD_SERVER::ENGINE::Engine(e, dbPath, logDBPath,
                                                 metaDBPath, metaDBSize);

  Document Request = Document(Type::kObjectType);
  auto &allocator = Request.GetAllocator();

  Request.AddMember("request", Value("get-trail"), allocator);
  Request.AddMember("request-ids", Value(Type::kArrayType), allocator);
  auto &IDList = Request["request-ids"];
  IDList.PushBack(1, allocator).PushBack(2, allocator);

  auto Response = Engine.handleGetTrailRequest(e, Request);

  StringBuffer sb;
  PrettyWriter<StringBuffer> writer(sb);
  Response.Accept(writer);
  LOG("handleGetTrailRequest Response JSON: " << sb.GetString())

}  // GetTrailRequest

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
