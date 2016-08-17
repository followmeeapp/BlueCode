//
//  main.cpp
//  PageRank
//
//  Created by Erik van der Tier on 9/01/15.
//  Copyright (c) 2015 Xy Group Llc. All rights reserved.
//
#include "xy_pagerank_old.h"
#include "xy_graph_writer.h"
#include "xy_lmdb_graph.h"
#include <boost/graph/graphviz.hpp>
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

int main(int argc, char** argv) {
  // setup command Line options
  options_description Desc(
      "\nCalculates personalised page rank.\n\nAllowed arguments");
  Desc.add_options()("help", "Produces this help message")(
      "target", value<unsigned long>(), "specify target vertex")(
      "probability", value<float>(),
      "specify teleport probability (default=0.2)")(
      "tolerance", value<float>(), "specify error tolerance (default=0.1)")(
      "in-edge", value<string>(), "specify input file path for edges")(
      "in-vertex", value<string>(), "specify input file path for vertexes")(
      "out", value<string>(), "specify output file path");

  variables_map VarMap;
  store(parse_command_line(argc, argv, Desc), VarMap);
  notify(VarMap);

  // parse options
  if (VarMap.count("help")) {
    cout << Desc << endl;
    return 1;
  }

  // set defaults of probabily and tolerance
  auto Probability = 0.2f;
  auto Tolerance = 0.1f;

  if (VarMap.count("probability"))
    Probability = VarMap["probability"].as<float>();
  if (VarMap.count("tolerance")) Tolerance = VarMap["tolerance"].as<float>();

  graph_t Graph;

  // default output stream is standard out
  ostream* DotOutput = &cout;

  if (VarMap.count("in-edge") && VarMap.count("in-vertex")) {
    // TODO: add exception handling around the file streams

    ifstream VertexFile(VarMap["in-vertex"].as<string>());
    ifstream EdgeFile(VarMap["in-edge"].as<string>());

    if (!VertexFile || !EdgeFile) {
      cout << "could not open input files!" << endl;
      return 1;
    }

    // all input and output streams are set, lets proceed!

    auto LoadStartTime = chrono::steady_clock::now();

    map<unsigned long, vertex_t> VertexIDMap;
    auto ItemCounter = 0l;
    for (string Line; getline(VertexFile, Line);) {
      auto VertexID = stoul(Line);

      auto Vertex = add_vertex(Graph);
      VertexIDMap[VertexID] = Vertex;
      // Graph[Vertex].label = to_string(VertexID);

      ItemCounter++;

      /*
      if (!(ItemCounter++ % 100000)) {
          cout << "processed " << ItemCounter-1 << "vertexes..." << endl;
      }
       */
      //            tie(VertexID, u) = VertexIDMap.insert(make_pair(VertexID,
      //            u));
    }

    cout << "processed " << ItemCounter << "vertices..." << endl;

    ItemCounter = 0;
    for (string Line; getline(EdgeFile, Line);) {
      boost::char_delimiters_separator<char> sep(false, "", ",");
      boost::tokenizer<> LineToks(Line, sep);
      auto TokenIt = LineToks.begin();

      auto EdgeID = stoul(*TokenIt++);
      auto VertexID = stoul(*TokenIt);

      add_edge(VertexIDMap[EdgeID], VertexIDMap[VertexID], Graph);

      ItemCounter++;
      /*if (!(ItemCounter++ % 100000)) {
          cout << "processed " << ItemCounter-1 << "edges..." << endl;
      }*/
    }
    cout << "processed " << ItemCounter << "edges..." << endl;

    auto LoadEndTime = chrono::steady_clock::now();
    auto LoadDuration = LoadEndTime - LoadStartTime;
    auto LoadDurationMS = chrono::duration<double, milli>(LoadDuration).count();
    cout << "Finished loading in " << LoadDurationMS << "\n";

  } else {
    cout << "no input files specified!" << endl;
    return 1;
  }

  std::map<vertex_t, vertex_property_t> VertexProperties;

  // ErrorPtr e;

  cout << "Starting page ranking algorithm!" << endl;

  map<string, string> GraphAttr, VertexAttr, EdgeAttr;
  GraphAttr["size"] = "3,3";
  GraphAttr["rankdir"] = "LR";
  GraphAttr["ratio"] = "fill";
  VertexAttr["shape"] = "record";

  typename VertexWriter<vertex_t>::writer_t VertexWriter =
      [&Graph, &VertexProperties](ostream& out, const vertex_t& v) {
        out << "[label=\"<f0>" << to_string(v + 1)
            << "| <f1> R=" << VertexProperties[v].rank
            << "| <f2> P=" << Graph[v].prio << "\"]";
      };
  typename EdgeWriter<edge_t>::writer_t EdgeWriter =
      [&Graph](ostream& out, const edge_t& e) {
        out << "[label=" << Graph[e].weight << "]";
      };

  //! if there is a target argument passed only page rank this target and write
  // out the resulting graph.
  if (VarMap.count("target")) {
    vertex_t TargetVertex = VarMap["target"].as<unsigned long>();
    XYPageRank(Graph, Probability, Tolerance, TargetVertex, VertexProperties,
               false);
    // if a file is specified set the output stream to a ofstream for the file
    try {
      if (VarMap.count("out")) {
        string OutputPath = VarMap["out"].as<string>() + string("_") +
                            to_string(VarMap["target"].as<unsigned long>()) +
                            string(".dot");
        DotOutput = new ofstream(OutputPath);
        DotOutput->exceptions(ofstream::failbit | ofstream::badbit);
      }
    } catch (const std::exception& e) {
      cout << "Could not open output file!" << endl;
      return 1;
    }

    writePageRankedGraph<graph_t, vertex_t, edge_t>(
        *DotOutput, Graph, TargetVertex, Probability, Tolerance, GraphAttr,
        VertexAttr, VertexWriter, EdgeWriter);
    if (VarMap.count("out")) {
      dynamic_cast<ofstream*>(DotOutput)->close();
      delete DotOutput;
    }

  } else {  //! if no target argument was specified iterate through the entire
    // graph and rank for each node as target

    typename graph_traits<graph_t>::vertex_iterator VertexIt, VertexEndIt;

    auto BatchStartTime = chrono::steady_clock::now();

    auto OutputFileIndex = 0;
    for (vertex_t i = 0; i < 2000; ++i) {
      cout << "processing node: " << OutputFileIndex << "\n";

      auto RankingStartTime = chrono::steady_clock::now();

      auto NumIterations =
          XYPageRank(Graph, Probability, 0.01, i, VertexProperties, false);

      auto RankingEndTime = chrono::steady_clock::now();
      auto RankingDuration = RankingEndTime - RankingStartTime;

      cout << "Finished with page ranking algorithm in "
           << chrono::duration<double, milli>(RankingDuration).count()
           << "miliseconds\n";

      // for now, only output on 'interesting graphs'
      if (false) {
        // if a file is specified set the output stream to an ofstream for the
        // file
        try {
          if (VarMap.count("out")) {
            string OutputPath = VarMap["out"].as<string>() + string("_") +
                                to_string(OutputFileIndex) + string(".dot");
            DotOutput = new ofstream(OutputPath);
            DotOutput->exceptions(ofstream::failbit | ofstream::badbit);
          }
        } catch (const std::exception& e) {
          cout << "Could not open output file!" << endl;
          return 1;
        }

        writePageRankedGraph<graph_t, vertex_t, edge_t>(
            *DotOutput, Graph, *VertexIt, Probability, Tolerance, GraphAttr,
            VertexAttr, VertexWriter, EdgeWriter);
        if (VarMap.count("out")) {
          dynamic_cast<ofstream*>(DotOutput)->close();
          delete DotOutput;
        }
      }
      //! brute force clean the entire graph
      // typename boost::graph_traits<graph_t>::vertex_iterator VertexIt2,
      //  VertexEndIt2;

      /*for (std::tie(VertexIt2, VertexEndIt2) = vertices(Graph);
           VertexIt2 != VertexEndIt2; ++VertexIt2) {
        Graph[*VertexIt2].prio = 0.f;
        Graph[*VertexIt2].rank = 0.f;
      }*/

      OutputFileIndex++;
      VertexProperties.clear();
    }

    auto BatchEndTime = chrono::steady_clock::now();
    auto BatchDuration = BatchEndTime - BatchStartTime;
    auto BatchDurationMS =
        chrono::duration<double, milli>(BatchDuration).count();
    cout << "Finished with batch in " << BatchDurationMS
         << "miliseconds, average: " << BatchDurationMS / (OutputFileIndex - 1)
         << "\n";
  }

  return 0;
}
