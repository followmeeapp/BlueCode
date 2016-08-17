//
//  //discovery_graph/graph/graph_test.cpp
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//

#include <gtest/gtest.h>
#include <stdio.h>
#include <chrono>
#include "discovery_engine/engine/engine.h"
#include "discovery_engine/cpp_lmdb/lmdb_database.h"
#include "discovery_engine/cpp_lmdb/lmdb_env.h"
#include "discovery_engine/cpp_lmdb/lmdb_cursor.h"
using namespace DISCOVERY_ENGINE::ENGINE;

namespace {

const auto SecondDuration = 1000;
const auto MinuteDuration = SecondDuration * 60;
const auto HourDuration = MinuteDuration * 60;

using std::string;

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
    string command = string("rm ") + dbPath + string("/*");

    popen(command.c_str(), "w");
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
  string dbPath = "/Users/erik/Documents/Suiron/Data/engine_test";
};

//__________________________________________________________________________________
//
// EngineErrorTest
//__________________________________________________________________________________

TEST_F(engine_test, EngineErrorTest) {
  ErrorPtr e;

  e.set(new engine_error_t(ERROR_CODE::UNKNOWN_DISC_TYPE));

  ASSERT_EQ("discovery type is unknown", e.message());
  ERROR_RESET(e);
  LMDBEnv EngineEnv = LMDBEnv(e, dbPath, 16, 4048);
  auto Engine = engine(e, EngineEnv);

}  // EngineErrorTest

//__________________________________________________________________________________
//
// InfoCmpSortTest (CardInfo & DeviceInfo Dup sorting
//__________________________________________________________________________________

TEST_F(engine_test, InfoCmpSortTest) {
  int EQ = 0;
  int LT = -1;
  int GT = 1;

  // struct { time, id }
  info_record val1 = {1, 1};
  info_record val2 = {1, 1};
  info_record val3 = {2, 1};
  info_record val4 = {1, 2};
  info_record val5 = {2, 2};

  MDB_val mdbVal1 = {sizeof(info_record), (void *)&val1};
  MDB_val mdbVal2 = {sizeof(info_record), (void *)&val2};
  MDB_val mdbVal3 = {sizeof(info_record), (void *)&val3};
  MDB_val mdbVal4 = {sizeof(info_record), (void *)&val4};
  MDB_val mdbVal5 = {sizeof(info_record), (void *)&val5};

  ASSERT_EQ(EQ, InfoCmp(&mdbVal1, &mdbVal2));
  // GT because higher time stamp is more recent, sorted before lower timestamp
  ASSERT_EQ(GT, InfoCmp(&mdbVal1, &mdbVal3));
  // LT because lower time stamp is later, sorted before higher timestamp
  ASSERT_EQ(LT, InfoCmp(&mdbVal3, &mdbVal1));
  // LT because: time stamps are equal but first id is lower than second id
  ASSERT_EQ(LT, InfoCmp(&mdbVal1, &mdbVal4));
  // GT because first timestapm is lower, thus more recent and has preference
  // over
  // id sorting, which is only a falback that should not be reached in real life
  ASSERT_EQ(GT, InfoCmp(&mdbVal1, &mdbVal5));

}  // EngineErrorTest

//__________________________________________________________________________________
//
// NewDevice
//__________________________________________________________________________________

TEST_F(engine_test, NewDevice) {
  ErrorPtr e;

  LMDBEnv EngineEnv = LMDBEnv(e, dbPath, 16, 4048);
  auto Engine = engine(e, EngineEnv);

  capnp::MallocMessageBuilder builder;
  capnp::MallocMessageBuilder ResponseBuilder;
  capnp::MallocMessageBuilder DiscResponseBuilder;

  // set CurrentTime as a time baseline for interactions in this test
  auto CurrentTime = now();

  // First setup onSendSyncRequest handler
  // We're creating a SyncResponse here which can be send back in a separate
  // transaction after we send the DiscoverRequest and gotten our
  // onSendSyncRequest call back
  auto SResponse = ResponseBuilder.initRoot<SyncResponse>();
  Engine.onSendSyncRequest([&ResponseBuilder, &SResponse, &CurrentTime](
      ErrorPtr &e, int64_t deviceID, SyncRequest::Reader request) {

    // get DiscoverInfo from request and validate
    auto DiscoverInfo = request.getDiscoverInfo();

    ASSERT_EQ(DiscoverInfo::Type::DISCOVER, DiscoverInfo.getType());
    // ASSUMPTION ERIK: LatestRecentCard is set to 0 on initial sync request
    ASSERT_EQ(0, request.getLatestRecentCard());
    ASSERT_EQ(true, request.getWantsCardsAdvertisedByDevice());

    // set up SyncResponse
    SResponse.setDiscoverInfo(DiscoverInfo);

    auto RecentCardList = SResponse.initRecentCards(2);
    RecentCardList[0].setId(100);
    RecentCardList[0].setTimestamp(CurrentTime - HourDuration);
    RecentCardList[1].setId(200);
    RecentCardList[1].setTimestamp(CurrentTime - HourDuration + 1);

    auto DiscoveredDeviceList = SResponse.initDiscoveredDevices(2);
    DiscoveredDeviceList[0].setId(1000);
    DiscoveredDeviceList[0].setTimestamp(CurrentTime - HourDuration);
    DiscoveredDeviceList[1].setId(2000);
    DiscoveredDeviceList[1].setTimestamp(CurrentTime - HourDuration);

    auto DeviceCardsList = SResponse.initCardsAdvertisedByDevice(2);
    DeviceCardsList.set(0, 10000);
    DeviceCardsList.set(1, 20000);

    // add new sync info
    // this could actually be calculated on the DiscoverD, but as a service
    // this is already done.
    auto SInfo = SResponse.initSyncInfo();
    SInfo.setLastCard(200);
    SInfo.setCardsVersion(1);
    SInfo.setLastDeviceUpdate(CurrentTime - HourDuration);

  });  // onSendSyncRequest

  // Set up onSendDiscoverResponse handler
  // in this test case it will be called from 'engine.handleSyncResponse'

  auto DAResponse = DiscResponseBuilder.initRoot<DiscoverAckResponse>();
  Engine.onSendDiscoverResponse(
      [&DiscResponseBuilder, &DAResponse, &CurrentTime](
          ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response) {

        auto Info = response.getDiscoverInfo();
        ASSERT_EQ(DiscoverInfo::Type::DISCOVER, Info.getType());
        ASSERT_GE(response.getTimestamp(), CurrentTime);
        ASSERT_EQ(0, response.getCards().size());

        // check on sync info
        // it should be same as that from our own sync response
        auto SInfo = response.getSyncInfo();
        ASSERT_EQ(200, SInfo.getLastCard());
        ASSERT_EQ(1, SInfo.getCardsVersion());
        ASSERT_EQ(CurrentTime - HourDuration, SInfo.getLastDeviceUpdate());

        DAResponse.setTimestamp(response.getTimestamp());
      });  // onSendDiscoveryResponse

  // call handelDiscoverRequest
  Engine.beginTransaction(
      e, [&Engine, &builder, &CurrentTime](ErrorPtr &e, Transaction &txn) {

        auto Request = builder.initRoot<DiscoverRequest>();
        auto DiscInfo = Request.initDiscoverInfo();
        // we don't set the time stamp as that's only for Ack
        // we don't set the requestID (in discoverInfo) only
        // for client, not testing that here
        DiscInfo.setType(DiscoverInfo::Type::DISCOVER);
        DiscInfo.setShouldSuppressSectionCreation(true);

        Engine.handleDiscoverRequest(e, 1, Request, txn);
        ASSERT_FALSE(e);
        Engine.commitTransaction(e, txn);
        ASSERT_FALSE(e);
      });

  // send the SyncResponse
  std::cout << e.message();
  Engine.beginTransaction(e,
                          [&Engine, &SResponse](ErrorPtr &e, Transaction &txn) {
                            Engine.handleSyncResponse(e, 1, SResponse, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  // send the SyncResponse
  std::cout << e.message();
  Engine.beginTransaction(e,
                          [&Engine, &SResponse](ErrorPtr &e, Transaction &txn) {
                            Engine.handleSyncResponse(e, 1, SResponse, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  // send the AckResponse
  Engine.beginTransaction(
      e, [&Engine, &DAResponse](ErrorPtr &e, Transaction &txn) {
        Engine.handleDiscoverAckResponse(e, 1, DAResponse, txn);
        ASSERT_FALSE(e);
        Engine.commitTransaction(e, txn);
        ASSERT_FALSE(e);
      });

}  // NewDevice

TEST_F(engine_test, DiscoverGraph) {
  ErrorPtr e;

  LMDBEnv EngineEnv = LMDBEnv(e, dbPath, 16, 1048);
  auto Engine = engine(e, EngineEnv);
  auto CurrentTime = now();
  capnp::MallocMessageBuilder builder;
  capnp::MallocMessageBuilder ResponseBuilder;
  capnp::MallocMessageBuilder DiscResponseBuilder;

  auto SResponse = ResponseBuilder.initRoot<SyncResponse>();
  Engine.onSendSyncRequest([&ResponseBuilder, &SResponse, &CurrentTime](
      ErrorPtr &e, int64_t deviceID, SyncRequest::Reader request) {
    auto Info = request.getDiscoverInfo();
    ASSERT_EQ(DiscoverInfo::Type::DISCOVER, Info.getType());
    ASSERT_EQ(0, request.getLatestRecentCard());
    ASSERT_EQ(true, request.getWantsCardsAdvertisedByDevice());
    SResponse.setDiscoverInfo(Info);

    auto RecentCardList = SResponse.initRecentCards(2);
    RecentCardList[0].setId(10);
    RecentCardList[0].setTimestamp(CurrentTime - 4599000);
    RecentCardList[1].setId(20);
    RecentCardList[1].setTimestamp(CurrentTime - 4599000 - 900001);
    auto DiscoveredDeviceList = SResponse.initDiscoveredDevices(2);
    DiscoveredDeviceList[0].setId(2);
    DiscoveredDeviceList[0].setTimestamp(20);
    DiscoveredDeviceList[1].setId(3);
    DiscoveredDeviceList[1].setTimestamp(30);
    auto DeviceCardsList = SResponse.initCardsAdvertisedByDevice(2);
    DeviceCardsList.set(0, 1);
    DeviceCardsList.set(1, 2);
  });

  auto DAResponse = DiscResponseBuilder.initRoot<DiscoverAckResponse>();
  Engine.onSendDiscoverResponse([&DiscResponseBuilder, &DAResponse](
      ErrorPtr &e, int64_t deviceID, DiscoverResponse::Reader response) {

    auto Info = response.getDiscoverInfo();
    ASSERT_EQ(DiscoverInfo::Type::DISCOVER, Info.getType());
    ASSERT_GE(now(), response.getTimestamp());

    LOG("discoverd cards for: " << deviceID << ": ");
    for (auto card : response.getCards()) {
      LOG("  card: " << card.getId() << ", " << card.getTimestamp())
    }

    DAResponse.setTimestamp(response.getTimestamp());
  });  // onSendDiscoveryResponse

  LOG("\nTEST:: Beginning Card Discovery Testing\n")

  LOG("TEST:: Send Discover Request for: 1\n")
  Engine.beginTransaction(
      e, [&Engine, &builder, &CurrentTime](ErrorPtr &e, Transaction &txn) {
        auto request = builder.initRoot<DiscoverRequest>();
        auto discInfo = request.initDiscoverInfo();
        discInfo.setType(DiscoverInfo::Type::DISCOVER);
        discInfo.setShouldSuppressSectionCreation(true);

        request.setTimestamp(CurrentTime);
        Engine.handleDiscoverRequest(e, 1, request, txn);
        ASSERT_FALSE(e);
        Engine.commitTransaction(e, txn);
        ASSERT_FALSE(e);
      });

  LOG("\nTEST:: Send Sync Request for 1:\n")

  Engine.beginTransaction(e,
                          [&Engine, &SResponse](ErrorPtr &e, Transaction &txn) {
                            Engine.handleSyncResponse(e, 1, SResponse, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  Engine.onSendSyncRequest([&ResponseBuilder, &SResponse](
      ErrorPtr &e, int64_t deviceID, SyncRequest::Reader request) {
    auto Info = request.getDiscoverInfo();
    ASSERT_EQ(DiscoverInfo::Type::DISCOVER, Info.getType());
    ASSERT_EQ(0, request.getLatestRecentCard());
    ASSERT_EQ(true, request.getWantsCardsAdvertisedByDevice());
    SResponse.setDiscoverInfo(Info);

    auto RecentCardList = SResponse.initRecentCards(2);
    RecentCardList[0].setId(30);
    RecentCardList[0].setTimestamp(now() - 4599000);
    RecentCardList[1].setId(40);
    RecentCardList[1].setTimestamp(now() - 4599000);
    auto DiscoveredDeviceList = SResponse.initDiscoveredDevices(3);
    DiscoveredDeviceList[0].setId(4);
    DiscoveredDeviceList[0].setTimestamp(20);
    DiscoveredDeviceList[1].setId(5);
    DiscoveredDeviceList[1].setTimestamp(30);
    DiscoveredDeviceList[2].setId(6);
    DiscoveredDeviceList[2].setTimestamp(30);
    auto DeviceCardsList = SResponse.initCardsAdvertisedByDevice(2);
    DeviceCardsList.set(0, 3);
    DeviceCardsList.set(1, 4);
  });

  LOG("\nTEST:: Send Discover Request for: 2\n")

  Engine.beginTransaction(e,
                          [&Engine, &builder](ErrorPtr &e, Transaction &txn) {
                            auto request = builder.initRoot<DiscoverRequest>();
                            auto discInfo = request.initDiscoverInfo();
                            discInfo.setType(DiscoverInfo::Type::DISCOVER);
                            discInfo.setShouldSuppressSectionCreation(false);

                            request.setTimestamp(20);
                            Engine.handleDiscoverRequest(e, 2, request, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  LOG("\nTEST:: Send Sync Request for 2:\n")

  Engine.beginTransaction(e,
                          [&Engine, &SResponse](ErrorPtr &e, Transaction &txn) {
                            Engine.handleSyncResponse(e, 2, SResponse, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  Engine.onSendSyncRequest([&ResponseBuilder, &SResponse](
      ErrorPtr &e, int64_t deviceID, SyncRequest::Reader request) {
    auto Info = request.getDiscoverInfo();
    ASSERT_EQ(DiscoverInfo::Type::DISCOVER, Info.getType());
    ASSERT_EQ(0, request.getLatestRecentCard());
    ASSERT_EQ(true, request.getWantsCardsAdvertisedByDevice());
    SResponse.setDiscoverInfo(Info);

    auto RecentCardList = SResponse.initRecentCards(2);
    RecentCardList[0].setId(50);
    RecentCardList[0].setTimestamp(now() - 4599000);
    RecentCardList[1].setId(60);
    RecentCardList[1].setTimestamp(now() - 4599000);
    auto DiscoveredDeviceList = SResponse.initDiscoveredDevices(3);
    DiscoveredDeviceList[0].setId(4);
    DiscoveredDeviceList[0].setTimestamp(20);
    DiscoveredDeviceList[1].setId(1);
    DiscoveredDeviceList[1].setTimestamp(30);
    DiscoveredDeviceList[2].setId(2);
    DiscoveredDeviceList[2].setTimestamp(40);
    auto DeviceCardsList = SResponse.initCardsAdvertisedByDevice(2);
    DeviceCardsList.set(0, 5);
    DeviceCardsList.set(1, 6);
  });

  LOG("\nTEST:: Send Discover Request for: 3\n")

  Engine.beginTransaction(e,
                          [&Engine, &builder](ErrorPtr &e, Transaction &txn) {
                            auto request = builder.initRoot<DiscoverRequest>();
                            auto discInfo = request.initDiscoverInfo();
                            discInfo.setType(DiscoverInfo::Type::DISCOVER);
                            discInfo.setShouldSuppressSectionCreation(false);

                            request.setTimestamp(20);
                            Engine.handleDiscoverRequest(e, 3, request, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  LOG("\nTEST:: Send Sync Request for 3:\n")

  Engine.beginTransaction(e,
                          [&Engine, &SResponse](ErrorPtr &e, Transaction &txn) {
                            Engine.handleSyncResponse(e, 3, SResponse, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  LOG("\nTEST:: Send Discovery Request for 1:\n")

  Engine.beginTransaction(e,
                          [&Engine, &builder](ErrorPtr &e, Transaction &txn) {
                            auto request = builder.initRoot<DiscoverRequest>();
                            auto discInfo = request.initDiscoverInfo();
                            discInfo.setType(DiscoverInfo::Type::DISCOVER);
                            discInfo.setShouldSuppressSectionCreation(true);

                            request.setTimestamp(20);
                            Engine.handleDiscoverRequest(e, 1, request, txn);
                            ASSERT_FALSE(e);
                            Engine.commitTransaction(e, txn);
                            ASSERT_FALSE(e);
                          });

  Engine.beginTransaction(
      e, [&Engine, &DAResponse](ErrorPtr &e, Transaction &txn) {
        Engine.handleDiscoverAckResponse(e, 1, DAResponse, txn);
        ASSERT_FALSE(e);
        Engine.commitTransaction(e, txn);
        ASSERT_FALSE(e);
      });

  ERROR_RESET(e);

}  // discover graph test

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
