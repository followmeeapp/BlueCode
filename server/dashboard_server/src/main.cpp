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

#include "aeron/util/CommandOptionParser.h"

#include "capnproto/capnp/message.h"
#include "capnproto/capnp/serialize-packed.h"
#include "capnproto/kj/exception.h"

#include "schema/rpc.capnp.h"
#include "uWebSockets/uWS.h"
#include "cpp_lmdb/lmdb_env.h"

#include "config.h"
#include "generic.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "dashboard_server/engine/engine.h"

using namespace std;
using namespace std::chrono;
using namespace aeron::util;
using namespace LMDB;
using namespace rapidjson;

using namespace DASHBOARD_SERVER::ENGINE;

namespace DASHBOARD_SERVER {
std::atomic<bool> running(true);

static const char optHelp = 'h';

static const char optServerPort = 'p';
static const char optDBPath = 'd';
static const char optLogDBPath = 'l';

// LMDB meta database options
static const char optMetaDBPath = 'm';
static const char optMetaDBSize = 's';

struct Settings {
  string dbPath = DASHBOARD_SERVER::CONFIG::DEFAULT_DB_PATH;
  string logDBPath = DASHBOARD_SERVER::CONFIG::DEFAULT_LOG_DB_PATH;
  string metaDBPath = DASHBOARD_SERVER::CONFIG::DEFAULT_META_DB_PATH;
  int32_t metaDBSize = DASHBOARD_SERVER::CONFIG::DEFAULT_META_DB_SIZE;
  int32_t serverPort = DASHBOARD_SERVER::CONFIG::DEFAULT_SERVER_PORT;
};

int connections = 0;

static const char *GET_TRAIL_REQ = "get-trail";

auto parseCmdLine(int argc, char **argv) -> Settings {
  CommandOptionParser cp;
  cp.addOption(CommandOption(optHelp, 0, 0,
                             "                Displays help information."));
  cp.addOption(CommandOption(optServerPort, 1, 1,
                             "port         dashboard server port."));
  cp.addOption(CommandOption(optDBPath, 1, 1,
                             "db_path         Directory for event data."));
  cp.addOption(CommandOption(optLogDBPath, 1, 1,
                             "log_db_path         Directory for event data."));

  cp.addOption(CommandOption(optMetaDBPath, 1, 1,
                             "meta_db_path         Directory for meta data."));
  cp.addOption(CommandOption(
      optMetaDBSize, 1, 1,
      "meta_db_size         meta data file size in MB. (Default = 4096MB)"));
  cp.parse(argc, argv);

  if (cp.getOption(optHelp).isPresent()) {
    cp.displayOptionsHelp(cout);
    exit(0);
  }

  Settings s;

  s.serverPort =
      cp.getOption(optServerPort).getParamAsInt(0, 1, INT32_MAX, s.serverPort);
  s.dbPath = cp.getOption(optDBPath).getParam(0, s.dbPath);
  s.logDBPath = cp.getOption(optLogDBPath).getParam(0, s.logDBPath);

  s.metaDBPath = cp.getOption(optMetaDBPath).getParam(0, s.metaDBPath);
  s.metaDBSize =
      cp.getOption(optMetaDBSize).getParamAsInt(0, 1, INT32_MAX, s.metaDBSize);
  return s;
}
}  // namespace DASHBOARD_SERVER

using namespace DASHBOARD_SERVER;

auto main(int argc, char **argv) -> int {
  assert(sizeof(unsigned long long) == sizeof(void *) &&
         "Expect sizeof(unsigned long long) == sizeof(void *)");

  try {
    auto settings = parseCmdLine(argc, argv);

    cout << "Dashboard server" << endl;
    cout << "Listening at port: " << settings.serverPort << endl;

    ErrorPtr e;
    LMDBEnv MetaEnv = LMDBEnv(e, settings.metaDBPath, 16, settings.metaDBSize);

    auto LogEngine = Engine(e, settings.dbPath, settings.logDBPath,
                            settings.metaDBPath, settings.metaDBSize);

    uWS::Server server(settings.serverPort);

    server.onConnection([](uWS::ServerSocket socket) {
      cout << "[Connection] clients: " << ++connections << endl;
    });

    server.onMessage([&LogEngine](uWS::ServerSocket socket, const char *msg,
                                  size_t len, uWS::OpCode opCode) {
      ErrorPtr e;

      bool didError = false;
      bool needTocloseConnection = false;
      std::string ErrorStr = "";

      Document RequestJSON;

      char buffer[len];
      memcpy(buffer, msg, len);
      std::cout << "We got this buffer: " << buffer << std::endl;
      if (RequestJSON.ParseInsitu(buffer).HasParseError()) {
        return 1;
      }
      std::cout << "We parsed the json" << std::endl;
      auto &request = RequestJSON["request"];

      if (!request.IsString()) {
        return 1;
      }

      auto Request = request.GetString();
      std::cout << "Request: " << Request << std::endl;
      if (strcmp(Request, GET_TRAIL_REQ) == 0) {
        std::cout << "we have a 'get-trail' request" << std::endl;
        auto Response = LogEngine.handleGetTrailRequest(e, RequestJSON);
        
        
        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        Response.Accept(writer);
        socket.send((char*)sb.GetString(), sb.GetSize(), uWS::OpCode::TEXT);

        std::cout << sb.GetString() << std::endl;

      } else {
        std::cout << "Unknown request: " << Request << std::endl;
        return 1;
      }

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

  } catch (CommandOptionException &e) {
    cerr << "ERROR: " << e.what() << endl
         << endl;
    return -1;
  } catch (std::exception &e) {
    cerr << "FAILED: " << e.what() << " : " << endl;
    return -1;
  }

  return 0;
}

