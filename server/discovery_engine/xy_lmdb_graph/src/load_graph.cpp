//
//  load_graph.cpp
//  xy_lmdb_graph
//
//  Created by Erik van der Tier on 12/03/15.
//
//

#include "xy_lmdb_graph.h"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/tokenizer.hpp"
#include "boost/token_functions.hpp"
#include <chrono>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>

// using namespace std;
using namespace boost::program_options;
using namespace XYGraphEngine;
using std::string;
using std::ifstream;
using std::ostream;
using std::cout;

int main(int argc, char** argv) {
  // setup command Line options
  ErrorPtr e;
  string DBPath = "/Users/erik/Documents/Suiron/Data/graph";
  ifstream VertexFile;
  ifstream EdgeFile;

  options_description Desc(
      "\nStores the content of graph files in lmdb_graph.\n\nAllowed "
      "arguments");
  Desc.add_options()("help", "Produces this help message")(
      "in-edge", value<string>(), "specify input file path for edges")(
      "in-vertex", value<string>(), "specify input file path for vertexes")(
      "db-path", value<string>(), "specify database path");

  variables_map VarMap;
  store(parse_command_line(argc, argv, Desc), VarMap);
  notify(VarMap);

  // parse options
  if (VarMap.count("help")) {
    cout << Desc << "\n";
    return 1;
  }

  if (VarMap.count("db-path")) DBPath = VarMap["db-path"].as<string>();

  VertexFile.open(VarMap["in-vertex"].as<string>());
  EdgeFile.open(VarMap["in-edge"].as<string>());

  // VertexFile.open(argv[1]);
  // EdgeFile.open(argv[2]);

  if (!VertexFile || !EdgeFile) {
    cout << "could not open input files!\n";
    return 1;
  }

  if (true) {
    // string DBPath = argv[3];
    auto Graph = lmdb_graph<std::string, double>(e, DBPath, 8592);
    if (e) std::cout << e.get()->message() << std::endl;
    ON_ERROR_RETURN_VAL(e, 1);
    ERROR_RESET(e);

    ON_ERROR_RETURN_VAL(e, 1);
    ERROR_RESET(e);

    auto LoadStartTime = std::chrono::steady_clock::now();
    auto ItemCounter = 0l;
    {
      auto txn = Graph.beginTransaction(e);
      for (string Line; getline(VertexFile, Line);) {
        auto VertexID = stoul(Line);

        Graph.addVertex(e, VertexID, std::to_string(VertexID), txn);

        ON_ERROR_RETURN_VAL(e, 1);
        ItemCounter++;
      }
      Graph.commitTransaction(e, txn);
    }
    if (e) cout << "failed: " << e.get()->message();
    ON_ERROR_RETURN_VAL(e, 1);

    auto LoadEndTime = std::chrono::steady_clock::now();
    auto LoadDuration = LoadEndTime - LoadStartTime;

    cout << "processed " << ItemCounter << "vertices...\n";
    cout << "in: "
         << std::chrono::duration<double, std::milli>(LoadDuration).count()
         << " ms\n";
    ERROR_RESET(e);

    LoadStartTime = std::chrono::steady_clock::now();

    std::map<uint64_t, std::vector<uint64_t>> InEdgesMap;
    std::map<uint64_t, std::vector<uint64_t>> OutEdgesMap;

    ItemCounter = 0;
    {
      auto txn = Graph.beginTransaction(e);
      if (e) cout << "failed: " << e.get()->message();
      ON_ERROR_RETURN_VAL(e, 1);
      for (string Line; getline(EdgeFile, Line);) {
        if (!(ItemCounter % 50000)) cout << ItemCounter << "\n";

        boost::char_delimiters_separator<char> sep(false, "", ",");
        boost::tokenizer<> LineToks(Line, sep);
        auto TokenIt = LineToks.begin();

        auto VertexID1 = stoul(*TokenIt++);
        auto VertexID2 = stoul(*TokenIt);
        Graph.addEdgeBulk(e, VertexID1, VertexID2, 0.0, txn);
        InEdgesMap[VertexID2].push_back(VertexID1);
        OutEdgesMap[VertexID1].push_back(VertexID2);

        if (e) cout << "failed: " << e.get()->message();
        ON_ERROR_RETURN_VAL(e, 1);
        ItemCounter++;
      }
      {
        // ifstream VertexFile(argv[1]);
        // VertexFile.open(VarMap["in-vertex"].as<string>());
        ifstream VertexFile(VarMap["in-vertex"].as<string>());

        ItemCounter = 0l;
        for (string Line; getline(VertexFile, Line);) {
          auto VertexID = stoul(Line);
          sort(begin(InEdgesMap[VertexID]), end(InEdgesMap[VertexID]));
          sort(begin(OutEdgesMap[VertexID]), end(OutEdgesMap[VertexID]));
          Graph.AjdListUpdate(e, VertexID, InEdgesMap[VertexID],
                              OutEdgesMap[VertexID], txn);

          if (e) cout << "failed: " << e.get()->message();
          ON_ERROR_RETURN_VAL(e, 1);
          ItemCounter++;
        }
      }
      Graph.commitTransaction(e, txn);
    }
    if (e) cout << "failed: " << e.get()->message();
    ON_ERROR_RETURN_VAL(e, 1);
    LoadEndTime = std::chrono::steady_clock::now();
    LoadDuration = LoadEndTime - LoadStartTime;

    cout << "processed " << ItemCounter << "edges...\n";
    cout << "in: "
         << std::chrono::duration<double, std::milli>(LoadDuration).count()
         << " ms\n";

  } else {
    cout << "no input files specified!\n";
    return 1;
  }
}