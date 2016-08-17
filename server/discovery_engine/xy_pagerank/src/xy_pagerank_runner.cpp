//
//  xy_pagerank_runner.cpp
//  xy_pagerank
//
//  Created by Erik van der Tier on 12/03/15.
//  Copyright (c) 2015 Xy Group Llc. All rights reserved.
//

#include "xy_pagerank.h"
#include "xy_graph_writer.h"
#include "xy_lmdb_graph.h"
//#include <boost/graph/graphviz.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <chrono>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>

using namespace std;
using namespace boost::graph;
using namespace boost::program_options;
using namespace XYGraphEngine;

#define XY_PAGERANK_USE_BOOST_GPAPH 0

int main(int argc, char **argv) {
  // setup command Line options
  options_description Desc(
      "\nCalculates personalised page rank.\n\nAllowed arguments");
  Desc.add_options()("help", "Produces this help message")(
      "target", value<unsigned long>(), "specify target vertex")(
      "num", value<unsigned long>(), "process the first <num> vertices")(
      "probability", value<float>(),
      "specify teleport probability (default=0.2)")(
      "tolerance", value<float>(), "specify error tolerance (default=0.1)")(
      "db-path", value<string>(), "specify database path")(
      "out", value<string>(), "specify output file path");

  variables_map VarMap;
  store(parse_command_line(argc, argv, Desc), VarMap);
  notify(VarMap);

  // parse options
  if (VarMap.count("help")) {
    cout << Desc << endl;
    return 1;
  }
  string DBPath = "/Users/erik/Documents/Suiron/Data/graph2";
  if (VarMap.count("db-path")) DBPath = VarMap["db-path"].as<string>();
  // set defaults of probabily and tolerance
  auto Probability = 0.2f;
  auto Tolerance = 0.01f;

  if (VarMap.count("probability"))
    Probability = VarMap["probability"].as<float>();
  if (VarMap.count("tolerance")) Tolerance = VarMap["tolerance"].as<float>();

  // default output stream is standard out
  ostream *DotOutput = &cout;

  std::map<uint64_t, vertex_property_t> VertexProperties;

  ErrorPtr e;

  auto Graph = lmdb_graph<std::string, double>(
      e, "/Users/erik/Documents/Suiron/Data/graph", 8192);

  if (e) std::cout << e.get()->message() << std::endl;
  ON_ERROR_RETURN_VAL(e, 1);
  ERROR_RESET(e);

  map<string, string> GraphAttr, VertexAttr, EdgeAttr;
  GraphAttr["size"] = "3,3";
  GraphAttr["rankdir"] = "LR";
  GraphAttr["ratio"] = "fill";
  VertexAttr["shape"] = "record";
  auto BatchStartTime = chrono::steady_clock::now();

  //! if a target argument is passed only page rank this target and write
  // out the resulting graph.
  if (VarMap.count("target")) {
    uint64_t Target =
        static_cast<uint64_t>(VarMap["target"].as<unsigned long>());

    ERROR_RESET(e);
    auto txn = Graph.beginTransaction(e, MDB_RDONLY);
    ON_ERROR_RETURN_VAL(e, 1);

    auto NumIterations = XYPageRank(Graph, Probability, Tolerance, Target,
                                    VertexProperties, false, txn);
    ON_ERROR_RETURN_VAL(e, 1);

    std::cout << "Number of iterations: " << NumIterations << "\n";

    ERROR_RESET(e);
    Graph.commitTransaction(e, txn);
    ON_ERROR_RETURN_VAL(e, 1);
  }

  //! if the num argument is given or no --num and no --target run a batch
  uint64_t NumVertices = 0;
  if (VarMap.count("num")) NumVertices = VarMap["num"].as<unsigned long>();
  if (VarMap.count("num") ||
      (!VarMap.count("num") && !VarMap.count("target"))) {
    auto OutputFileIndex = 0;

    ERROR_RESET(e);
    // auto txn = Graph.beginTransaction(e);
    //
    ERROR_RESET(e);
    Graph.forEachVertex(
        e, [&Graph, &NumVertices, &Probability, &Tolerance, &VertexProperties,
            &OutputFileIndex](ErrorPtr &e, const uint64_t *vertexID,
                              const DBVal *properties, LMDB::Transaction &txn) {

          auto RankingStartTime = chrono::steady_clock::now();
          cout << "processing node: " << *vertexID << "\n";

          auto NumIterations =
              XYPageRank(Graph, Probability, Tolerance, *vertexID,
                         VertexProperties, false, txn);
          auto RankingEndTime = chrono::steady_clock::now();
          auto RankingDuration = RankingEndTime - RankingStartTime;

          cout << "done: touched " << VertexProperties.size() << " vertices.\n";
          cout << "Number of iterations: " << NumIterations << "\n";
          cout << "Finished with page ranking algorithm in "
               << chrono::duration<double, milli>(RankingDuration).count()
               << "miliseconds\n";
          OutputFileIndex++;
          VertexProperties.clear();

          if (NumVertices > 0 && OutputFileIndex == NumVertices) {
            e.set(new error_t{MDB_NOTFOUND});
            return;
          }
        });
    if (e)
      if (e.get()->code != MDB_NOTFOUND) ON_ERROR_RETURN_VAL(e, 1);

    ERROR_RESET(e);
    // Graph.commitTransaction(e, txn);
    ON_ERROR_RETURN_VAL(e, 1);

    auto BatchEndTime = chrono::steady_clock::now();
    auto BatchDuration = BatchEndTime - BatchStartTime;
    auto BatchDurationMS =
        chrono::duration<double, milli>(BatchDuration).count();
    cout << "Finished with batch in " << BatchDurationMS
         << "miliseconds, average: " << BatchDurationMS / (OutputFileIndex - 1)
         << "\n";

    // if a file is specified set the output stream to a ofstream for the file
    try {
      if (VarMap.count("out")) {
        string OutputPath = VarMap["out"].as<string>() + string("_") +
                            to_string(VarMap["target"].as<unsigned long>()) +
                            string(".dot");
        DotOutput = new ofstream(OutputPath);
        DotOutput->exceptions(ofstream::failbit | ofstream::badbit);
      }
    } catch (const std::exception &e) {
      cout << "Could not open output file!" << endl;
      return 1;
    }
  }
  std::cout << Graph.getEnv().getVersionString();
}