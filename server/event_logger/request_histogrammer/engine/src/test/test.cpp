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
#include "event_logger/event_logger.h"
#include "event_logger/request_histogrammer/engine/engine.h"
#include "event_logger/request_histogrammer/engine/generic.h"
using namespace EVENT_LOGGER::REQUEST_HISTOGRAMMER::ENGINE;

namespace {

const auto NanoSec = 1;
const auto MilliSec = 1;
const auto Second = 1000;
const auto Minute = Second * 60;
const auto Hour = Minute * 60;

using std::string;
using namespace LMDB;

// The fixture for testing class Foo.
class engine_test : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  engine_test() {
    // You can do set-up work for each test here.
    string logCommand = string("rm ") + dbPath + string("/log.tbd");
    string log2Command = string("rm ") + dbPath + string("/log/*");
    string metaCommand = string("rm ") + dbPath + string("/meta/*");
    string histogramCommand = string("rm ") + dbPath + string("/histogram/*");
    system(logCommand.c_str());
    system(log2Command.c_str());
    system(metaCommand.c_str());
    system(histogramCommand.c_str());
    ErrorPtr e;

    trailEnv_ = std::make_shared<LMDB::LMDBEnv>(e, metaDBPath, 16, metaDBSize);
    ON_ERROR_RETURN(e);
    histogramEnv_ = std::make_shared<LMDB::LMDBEnv>(e, histogramDBPath, 16,
                                                    histogramDBSize);
    ON_ERROR_RETURN(e);
    metaDB_ = trailEnv_->openDatabase<DBVal, DBVal>(e, "meta", 0);
    ON_ERROR_RETURN(e);
    metaDB_->setKeyCompareFunction(e, EVENT_LOGGER::TraceKeyCmp);
    ON_ERROR_RETURN(e);
    requestUUIDDB_ = trailEnv_->openDatabase<DBVal, DBVal>(e, "request_uuid",
                                                           MDB_INTEGERKEY);
    ON_ERROR_RETURN(e);
    histogramDB_ = histogramEnv_->openDatabase<DBVal, DBVal>(e, "histogram",
                                                             MDB_INTEGERKEY);
    ON_ERROR_RETURN(e);
    statusDB_ =
        histogramEnv_->openDatabase<DBVal, DBVal>(e, "status", MDB_INTEGERKEY);
    ON_ERROR_RETURN(e);
  }

  virtual ~engine_test() {
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
  string dbPath = "/Users/erik/Documents/Suiron/Data/event_log";
  string logDBPath = dbPath + "/log";
  string metaDBPath = dbPath + "/meta";
  size_t metaDBSize = 4096;
  string histogramDBPath = dbPath + "/histogram";
  size_t histogramDBSize = 4096;
  size_t maxHistogramValue = 1000 * 60;

  std::shared_ptr<LMDB::LMDBEnv> trailEnv_;
  std::shared_ptr<LMDB::LMDBEnv> histogramEnv_;

  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> metaDB_;
  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> requestUUIDDB_;
  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> histogramDB_;
  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> statusDB_;
};

//__________________________________________________________________________________
//
// NewDevice
//__________________________________________________________________________________

TEST_F(engine_test, NewDevice) {
  ErrorPtr e;

  // set CurrentTime as a time baseline for interactions in this test

  std::vector<uint64_t> Intervals = {1000, 1000 * 60, 1000 * 60 * 10};
  std::vector<uint64_t> IntervalsWindows = {10, 6, 2};
  auto HISTEngine = Engine(e, dbPath, metaDBSize, histogramDBSize,
                           maxHistogramValue, Intervals, IntervalsWindows);
  ASSERT_FALSE(e) << "HISTEngine constructor";
  ERROR_RESET(e);

  struct event_row {
    int64_t time;
    Trace::Event::Type Type;
    Trace::Event::Function Function;
  };
  struct trace_row {
    int64_t deviceID;
    int16_t requestID;
    int64_t time;
    uint64_t duration;
    std::vector<event_row> events;
  };

  std::vector<trace_row> TraceRows = {
      {1,
       1,
       1,
        10,
       {
        {11, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       2,
       250,
        20,
       {
        {270, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       3,
       500,
        30,
       {
        {530, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       4,
       750,
        10,
       {
        {760, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       5,
       1000,
        20,
       {
        {1020, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       6,
       1250,
        30,
       {
        {1280, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       7,
       1500,
        10,
       {
        {1510, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       8,
       1800,
        20,
       {
        {1820, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
      {1,
       9,
       2000,
        30,
       {
        {2030, Trace::Event::Type::BEGIN, Trace::Event::Function::READ_REQUEST},
       },
      },
  };

  capnp::MallocMessageBuilder traceBuilder;
  for (auto &traceRow : TraceRows) {
    Trace::Builder trace = traceBuilder.initRoot<Trace>();
    trace.setDeviceId(traceRow.deviceID);
    trace.setRequestId(traceRow.requestID);
    trace.setTimestamp(traceRow.time);
    trace.setDuration(traceRow.duration);
    std::cout << "trace timestamp " << traceRow.time << "\n";
    auto events = trace.initEvents(traceRow.events.size());
    auto i = 0;
    for (auto &event : traceRow.events) {
      events[i].setTimestamp(event.time);
      events[i].setType(event.Type);
      events[i].setFunction(event.Function);
      ++i;
    }

    HISTEngine.logTrail(e, trace);
    ASSERT_FALSE(e);
  }

  ERROR_RESET(e);
  HISTEngine.processHistograms(e);

  hdr_histogram *hist = nullptr;
  auto res = hdr_init(1, maxHistogramValue, 1, &hist);
  ASSERT_EQ(res, 0);
  uint8_t i = 0;
  auto txn = histogramEnv_->beginTransaction(e, MDB_RDONLY);
  {  // cursor scope
    auto histCur = histogramDB_->getCursor(e, txn);
    EVENT_LOGGER::histogram_key HistKey = {i, 0};

    auto HistRecord = histCur->getKeyGreaterEqual(e, HistKey);
    while (HistRecord) {
      ASSERT_FALSE(e);
      ASSERT_TRUE(HistRecord.is_initialized());

      auto HistKey =
          static_cast<EVENT_LOGGER::histogram_key *>(HistRecord->key.buffer());
      auto HistBuffer = static_cast<char *>(HistRecord->value.buffer());
      auto res = hdr_log_decode(&hist, HistBuffer, HistRecord->value.size());
      ASSERT_EQ(res, 0);

      std::cout << "Interval: " << Intervals[HistKey->interval]
                << " at position: " << HistKey->position << "\n";

      hdr_percentiles_print(hist, stdout, 5, 1.0, CLASSIC);
      hdr_reset(hist);
      ++i;
      HistRecord = histCur->getNext(e);
    }
  }  // cursor scope
}  // NewDevice

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
