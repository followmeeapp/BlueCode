//
//  //event_logger/engine/engine.h
//
//
//  Created by Erik van der Tier on 21/07/2016.
//
//

#ifndef EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_ENGINE_H_
#define EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_ENGINE_H_

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"

#include "cpp_lmdb/lmdb_env.h"

#include "schema/trace.capnp.h"
#include "event_logger/event_logger.h"
#include "hdr_histogram/hdr_histogram.h"
#include "hdr_histogram/hdr_histogram_log.h"
#include "generic.h"

#include <string>
#include <vector>

namespace EVENT_LOGGER {
namespace REQUEST_HISTOGRAMMER {
namespace ENGINE {

using namespace LMDB;

struct Settings {
  std::string dbPath;
  std::string logDBPath;
  std::string metaDBPath;
  std::string histogramDBPath;
  size_t metaDBSize;
  size_t histogramDBSize;
  size_t maxHistogramValue;
};

class Engine {
 public:
  Engine(LMDB::ErrorPtr &e, std::string dbPath, size_t metaDBSize,
         size_t histogramDBSize, size_t maxHistogramValue,
         std::vector<uint64_t> intervals,
         std::vector<uint64_t> intervalWindows);
  virtual ~Engine();

  void processHistograms(ErrorPtr &e);
  void logTrail(ErrorPtr &e, Trace::Reader trace);

 private:
  Settings settings_;
  std::vector<uint64_t> intervals_;
  std::vector<uint64_t> intervalWindows_;
  std::vector<uint64_t> intervalStart_;

  // vector of positions of the last interval (sliding window)
  std::vector<uint8_t> histTopPositions_;
  trace_key lastTraceKey_ = {0, 0};

  std::vector<hdr_histogram *> histograms_;

  std::shared_ptr<LMDB::LMDBEnv> trailEnv_;
  std::shared_ptr<LMDB::LMDBEnv> histogramEnv_;

  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> metaDB_;
  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> requestUUIDDB_;
  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> histogramDB_;
  LMDB::DatabasePtr<LMDB::DBVal, LMDB::DBVal> statusDB_;
};  // class engine

}  // namespace ENGINE
}  // namespace REQUEST_HISTOGRAMMER
}  // namespace EVENT_LOGGER

#endif  // EVENT_LOGGER_REQUEST_HISTOGRAMMER_ENGINE_ENGINE_H_
