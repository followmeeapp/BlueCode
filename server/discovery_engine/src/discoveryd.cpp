//
//  discoveryd.cpp
//  discovery_engine
//
//  Created by Erik van der Tier on 28/09/2016.
//
//

#include "discoveryd.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <thread>

#include "aeron/Aeron.h"
#include "aeron/FragmentAssembler.h"
#include "aeron/concurrent/BusySpinIdleStrategy.h"
#include "aeron/util/CommandOptionParser.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"

#include "cpp_lmdb/lmdb_env.h"  // TODO: move error stuff to separate file

#include "schema/rpc.capnp.h"

#include "clock/clock.h"

#include "config.h"

using namespace std;
using namespace std::chrono;
using namespace aeron::util;
using namespace aeron;
using namespace DISCOVERY_ENGINE::ENGINE;
using namespace LMDB;

auto make_reader(MDB_val data) -> capnp::FlatArrayMessageReader {
  assert(data.mv_size % 8 == 0 &&
         "expected data.mv_size to be a multiple of 8");

  kj::ArrayPtr<const capnp::word> view((const capnp::word *)data.mv_data,
                                       data.mv_size / 8);
  return capnp::FlatArrayMessageReader(view);
}
