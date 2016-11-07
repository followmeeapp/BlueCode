//
//  //event_logger/graph/graph.cpp
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//
#include "engine.h"
#include "event_policy.h"
#include <uuid/uuid.h>
#include "generic.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"
#include "traildb-cpp/TrailDBConstructor.h"

#include "schema/trace.capnp.h"

namespace EVENT_LOGGER {
namespace REQUEST_HISTOGRAMMER {
namespace ENGINE {

using namespace LMDB;
using namespace EVENT_LOGGER;

Engine::Engine(ErrorPtr &e, std::string dbPath, size_t metaDBSize,
               size_t histogramDBSize, size_t maxHistogramValue,
               std::vector<uint64_t> intervals,
               std::vector<uint64_t> intervalWindows) {
  LOG("BEGIN: Engine constructor")
  LOG("dbPath: " << dbPath)
  LOG("metaDBSize: " << metaDBSize)
  LOG("histogramDBSize: " << histogramDBSize)
  LOG("maxHistogramValue: " << maxHistogramValue)

  ASSERT_ERROR_RESET(e);

  intervals_ = intervals;
  intervalWindows_ = intervalWindows;

  settings_.dbPath = dbPath;
  settings_.logDBPath = dbPath + "/log";
  settings_.metaDBPath = dbPath + "/log";
  settings_.histogramDBPath = dbPath + "/histogram";
  settings_.metaDBSize = metaDBSize;
  settings_.histogramDBSize = histogramDBSize;
  settings_.maxHistogramValue = maxHistogramValue;

  trailEnv_ =
      std::make_shared<LMDB::LMDBEnv>(e, settings_.metaDBPath, 16, metaDBSize);
  ON_ERROR_RETURN(e);
  histogramEnv_ = std::make_shared<LMDB::LMDBEnv>(
      e, settings_.histogramDBPath, 16, settings_.histogramDBSize);
  ON_ERROR_RETURN(e);
  metaDB_ = trailEnv_->openDatabase<DBVal, DBVal>(e, "meta", 0);
  ON_ERROR_RETURN(e);
  metaDB_->setKeyCompareFunction(e, TraceKeyCmp);
  ON_ERROR_RETURN(e);
  requestUUIDDB_ =
      trailEnv_->openDatabase<DBVal, DBVal>(e, "request_uuid", MDB_INTEGERKEY);
  ON_ERROR_RETURN(e);
  histogramDB_ = histogramEnv_->openDatabase<DBVal, DBVal>(e, "histogram", 0);
  ON_ERROR_RETURN(e);
  histogramDB_->setKeyCompareFunction(e, HistogramKeyCmp);
  ON_ERROR_RETURN(e);

  statusDB_ =
      histogramEnv_->openDatabase<DBVal, DBVal>(e, "status", MDB_INTEGERKEY);
  ON_ERROR_RETURN(e);

  LOG("Succesfully opened databases")

  // load lastTraceKey_ from statusDB_
  auto LastKey = statusDB_->getCopy(e, 1);
  ON_ERROR_RETURN(e);
  if (LastKey) lastTraceKey_ = *static_cast<trace_key *>(LastKey->buffer());

  LOG("Loaded LastKey from statusDB, with Time: "
      << lastTraceKey_.time << " requestType: " << lastTraceKey_.request_type)

  // initialize histTopPositions
  histTopPositions_ = std::vector<uint8_t>(intervals_.size(), 0);
  // read stored values from previous run
  auto HistTopPositionRecord = statusDB_->getCopy(e, 2);
  ON_ERROR_RETURN(e);

  LOG("Loaded top positions from previous run")

  // initialise interval progress timers
  // initialise historgram vector
  auto i = 0;
  for (auto interval : intervals_) {
    hdr_histogram *histogram = nullptr;
    auto res = hdr_init(1, settings_.maxHistogramValue, 1, &histogram);
    if (res != 0) {
      ERROR_SET_CODE_RETURN(e, res)
    }
    histograms_.push_back(histogram);
    intervalStart_.push_back(lastTraceKey_.time);
    if (HistTopPositionRecord)
      histTopPositions_[i] =
          static_cast<uint8_t *>(HistTopPositionRecord->buffer())[i];
  }

  LOG("END: Engine constructor")
}

Engine::~Engine() {
  for (auto histogram : histograms_) {
    free(histogram);
  }
}

/**
 Process a histogram

 @param e error object
 */
void Engine::processHistograms(ErrorPtr &e) {
  ASSERT_ERROR_RESET(e);
  LOG("Start processingHistograms from time: "
      << lastTraceKey_.time << " requestType: " << lastTraceKey_.request_type)
  auto txn = trailEnv_->beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN(e);
  auto histTxn = histogramEnv_->beginTransaction(e);
  ON_ERROR_RETURN(e);

  {  // cursor scope
    auto metaCursor = metaDB_->getCursor(e, txn);
    ON_ERROR_RETURN(e);

    auto nextTraceKey = lastTraceKey_;
    // make sure we'll get a value later than the last

    ++nextTraceKey.time;
    nextTraceKey.request_type = 0;

    // perhaps we need to deal with back processing old traces at startup
    auto LastTraceRecord =
        metaCursor->getFirst(e);  // getKeyGreaterEqual(e, nextTraceKey);
    ON_ERROR_RETURN(e);
    if (!LastTraceRecord) return;

    do {
      lastTraceKey_ = *static_cast<trace_key *>(LastTraceRecord->key.buffer());
      auto LastTrace = make_reader(LastTraceRecord->value).getRoot<Trace>();
      // shorter intervals just roll over and start the next one

      auto EventTimeStamp = LastTrace.getTimestamp();
      auto EventDuration = LastTrace.getDuration();

      LOG("Processing event of type: " << lastTraceKey_.request_type
                                       << " time: " << lastTraceKey_.time)

      uint8_t i = 0;  // interval counter
      for (auto interval : intervals_) {
        // roll over finished intervals
        if (EventTimeStamp >= intervalStart_[i] + interval) {
          // close and store histogram
          char *HistogramBuffer = nullptr;
          size_t len = 0;

          if (auto res = hdr_log_encode_len(histograms_[i], &HistogramBuffer,
                                            &len) != 0) {
            ERROR_SET_CODE_RETURN(e, res)
          }
          histTopPositions_[i] = ++histTopPositions_[i] % intervalWindows_[i];

          histogram_key HistKey = {i, histTopPositions_[i]};
          auto HistRecord = DBVal(len, HistogramBuffer);
          histogramDB_->add(e, HistKey, HistRecord, histTxn, true);

          // reset histogram for next cycle
          hdr_reset(histograms_[i]);
          intervalStart_[i] = EventTimeStamp;
          LOG("Rolling Interval: " << intervals_[i] << "at: " << EventTimeStamp)
        }
        // do histogram stuff

        if (!hdr_record_value(histograms_[i], EventDuration)) {
          ERROR_SET_CODE_RETURN(e, -10)  // TODO: add error message
        }
        LOG("Recorded value: " << EventDuration
                               << " for interval: " << interval)
        ++i;
      }  // for (auto interval : intervals_)
      LastTraceRecord = metaCursor->getNext(e);
      ON_ERROR_RETURN(e);
    } while (LastTraceRecord);
  }  // end cursor scope

  // store last processed trace
  statusDB_->add(e, 1, lastTraceKey_, histTxn, true);
  ON_ERROR_RETURN(e);

  LOG("Succesfully stored last trace of type: "
      << lastTraceKey_.request_type << " time: " << lastTraceKey_.time)

  // store current histogram top positions.
  auto PositionsBuffer = DBVal(histTopPositions_.size() * sizeof(uint8_t),
                               histTopPositions_.data());
  statusDB_->add(e, 2, PositionsBuffer, histTxn, true);
  ON_ERROR_RETURN(e);

  LOG("Succesfully added current top positions")

  histogramEnv_->commitTransaction(e, histTxn);
  ON_ERROR_RETURN(e);
  trailEnv_->commitTransaction(e, txn);
}

// helper for testing only, should be either simulated or used from the logger
void Engine::logTrail(ErrorPtr &e, Trace::Reader trace) {
  ASSERT_ERROR_RESET(e);
  LOG("BEGIN: logTrail")
  capnp::MallocMessageBuilder builder;
  
  auto RequestID = trace.getRequestId();
  uuid_t id;
  char *StringUUID = new char[37];
  auto TraceTimeStamp = trace.getTimestamp();
  
  // record trace events in metaDB
  std::vector<std::string> Fields{"Type", "Function"};
  
  LOG("before constructing trail")
  TrailDBConstructor<EventPolicy> TrailDB(settings_.logDBPath, Fields);
  LOG("after constructing trail")
  
  for (auto event : trace.getEvents()) {
    TrailDB.Add(StringUUID, event.getTimestamp() - TraceTimeStamp, event);
  }
  TrailDB.Finalize();
  
  // record trace in metaDB
  
  {  // transaction scope
    auto txn = trailEnv_->beginTransaction(e);
    ON_ERROR_RETURN(e);
    
    uuid_generate(id);
    uuid_unparse(id, StringUUID);
    LOG("Generated UUID: " << StringUUID)
    auto UUIDRecord = DBVal(37, StringUUID);
    requestUUIDDB_->add(e, RequestID, UUIDRecord, txn, false);
    ON_ERROR_RETURN(e);
    
    auto TraceReq = builder.initRoot<Trace>();
    TraceReq.setTimestamp(TraceTimeStamp);
    TraceReq.setDeviceId(trace.getDeviceId());
    TraceReq.setRequestId(trace.getRequestId());
    TraceReq.setDuration(trace.getDuration());
    TraceReq.setError(trace.getError());
    TraceReq.setStackTrace(trace.getStackTrace());
    TraceReq.setRequestData(trace.getRequestData());
    auto NumTraces = trace.getEvents().size();
    if (NumTraces == 0) {
    }
    
    auto RequestType = trace.getEvents()[0].getFunction();
    
    kj::Array<capnp::word> words = capnp::messageToFlatArray(builder);
    kj::ArrayPtr<kj::byte> bytes = words.asBytes();
    
    auto MetaRecord = DBVal(bytes.size(), bytes.begin());
    
    trace_key TraceKey = {TraceTimeStamp, static_cast<int16_t>(RequestType)};
    metaDB_->add(e, TraceKey, MetaRecord, txn, false);
    ON_ERROR_RETURN(e);
    
    trailEnv_->commitTransaction(e, txn);
    ON_ERROR_RETURN(e);
  }  // end transaction scope
  
}

}  // namespace ENGINE
}  // namespace REQUEST_HISTOGRAMMER
}  // namespace EVENT_LOGGER
