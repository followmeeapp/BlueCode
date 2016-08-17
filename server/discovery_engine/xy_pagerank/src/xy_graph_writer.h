//
//  XYGraphWriter.h
//  PageRank
//
//  Created by Erik van der Tier on 30/01/15.
//  Copyright (c) 2015 Xy Group Llc. All rights reserved.
//

#ifndef XYGRAPHENGINE_XY_GRAPH_WRITER_H_
#define XYGRAPHENGINE_XY_GRAPH_WRITER_H_

namespace XYGraphEngine {

#include <map>

using std::tie;
using std::cout;
using std::function;
using std::endl;
using std::map;
using boost::graph_traits;
using boost::adjacency_list;

// function object types for writing out vertex and edges.
template <typename V>
struct VertexWriter {
  typedef function<void(std::ostream &out, const V &v)> writer_t;
};
template <typename E>
struct EdgeWriter {
  typedef function<void(std::ostream &out, const E &e)> writer_t;
};

// Writes out .dot format fragment for a vertex and calls itself recursively
// for its in-edges that have a ranking higher than probability * tolerance
template <typename G, typename V, typename E>
void writePageRankedVertex(std::ostream &out, G &graph, V target,
                           float probability, float tolerance,
                           std::map<V, int> &processedVertexes,
                           typename VertexWriter<V>::writer_t vertexWriter,
                           typename EdgeWriter<E>::writer_t edgeWriter) {
  out << target;
  vertexWriter(out, target);
  out << ";" << endl;

  typename graph_traits<G>::in_edge_iterator InEdgeIt, InEdgeEndIt;

  for (tie(InEdgeIt, InEdgeEndIt) = in_edges(target, graph);
       InEdgeIt != InEdgeEndIt; ++InEdgeIt) {
    V InNeighbour = source(*InEdgeIt, graph);
    if (graph[InNeighbour].rank > probability * tolerance) {
      out << InNeighbour << "->" << target;

      edgeWriter(out, *InEdgeIt);

      out << ";" << endl;

      if (!processedVertexes[InNeighbour]) {
        processedVertexes[InNeighbour] = 1;
        writePageRankedVertex<G, V, E>(out, graph, InNeighbour, probability,
                                       tolerance, processedVertexes,
                                       vertexWriter, edgeWriter);
      }
    }
  }
}

// Writes out .dot format header and calls writePAgeRankedVertex for the target
// vertex.
template <typename G, typename V, typename E>
void writePageRankedGraph(std::ostream &out, G &graph, V target,
                          float probability, float tolerance,
                          const std::map<std::string, std::string> &graphAttrs,
                          const std::map<std::string, std::string> &vertexAttrs,
                          typename VertexWriter<V>::writer_t vertexWriter,
                          typename EdgeWriter<E>::writer_t edgeWriter) {
  out << "digraph G {" << endl;
  out << "    graph [" << endl;
  out << "           ";

  for (auto &GraphAttr : graphAttrs) {
    out << (GraphAttr != *graphAttrs.begin() ? ", " : "");
    out << GraphAttr.first << "=";

    if (GraphAttr.second.find(",") != std::string::npos)

      out << "\"" << GraphAttr.second << "\"";

    else
      out << GraphAttr.second;
  }

  out << "];" << endl
      << "    node [" << endl;
  out << "           ";

  for (auto &VertexAttr : vertexAttrs) {
    out << (VertexAttr != *vertexAttrs.begin() ? ", " : "");
    out << VertexAttr.first << "=";
    if (VertexAttr.second.find(",") != std::string::npos)
      out << "\"" << VertexAttr.second << "\"";
    else
      out << VertexAttr.second;
  }
  out << "];" << endl;

  map<V, int> ProcessedVertexes;
  // std::map<E, int> ProcessedEdges;
  ProcessedVertexes[target] = 1;

  writePageRankedVertex<G, V, E>(out, graph, target, probability, tolerance,
                                 ProcessedVertexes, vertexWriter, edgeWriter);

  out << "}" << endl;
}

}  // namespace XYGraphEngine

#endif
