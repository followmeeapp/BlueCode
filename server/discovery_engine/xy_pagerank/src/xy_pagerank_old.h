/*! \file XYPageRank.h
 \brief Implementation of a templated function that calculates the 'personalised
 page rank' for a specified target vertex.

 Created by Erik van der Tier on 10/01/15.
 Copyright (c) 2015 Xy Group Llc. All rights reserved.
*/

#ifndef XYGRAPHENGINE_XY_PAGERANK_H_
#define XYGRAPHENGINE_XY_PAGERANK_H_

#include "xy_pagerank_config.h"

#include <iostream>  // for std::cout
#include <utility>   // for std::pair
#include <boost/graph/adjacency_list.hpp>
#include <functional>

#if XY_PAGERANK_USE_XYPRIOQUEUE
#include "xy_priority_queue.h"
#else
#include <boost/heap/fibonacci_heap.hpp>
#endif

namespace XYGraphEngine {

using std::tie;
using std::cout;
using std::function;
using boost::graph_traits;
using boost::adjacency_list;
#if !XY_PAGERANK_USE_XYPRIOQUEUE
using boost::heap::fibonacci_heap;
#endif

// TODO: all the types below here (from edge_property_t to vertex_property_t)
// should move out of
// this file. The algorithm should use 'dynamic properties' and add them to any
// graph that is passed in. I leave them in here for now to make the dependency
// explicit.

struct vertex_property_t;

struct edge_property_t {
  float weight = 0.0;
  float wOutDegree = -1.0;
};

// Graph type with custom vertex and edge properties: vProp_t and eProp_t
typedef adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                       vertex_property_t, edge_property_t> graph_t;

// Type of the vertices and edges
typedef typename graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef typename graph_traits<graph_t>::edge_descriptor edge_t;

// Type of the priority queue elements. T
template <typename T>
struct prio_q_element_t {
  T element;
  float prio;
  bool operator==(const prio_q_element_t<T> &rhv) {
    return element == rhv.element;
  }
};

typedef function<bool(const prio_q_element_t<vertex_t> &n1,
                      const prio_q_element_t<vertex_t> &n2)> vertex_compare_t;
/*! Vertex property struct.
 \brief This defines the properties we store per vertex in the graph. There are
 two 'technical' properties: mHandel and v.

 handle stores a handle to a vertex_t from the fibonacci heap in order to refer
 back to the elements in the PriorityQueue. This handle is needed to update
 queue for the new priority.
 v stores the vertex descriptor of the vertex that 'owns' these properties.
 This is needed to retrieve the vertex after getting the properties out of the
 Priorityqueue. The property struct needs to be stored in the queue because the
 queue needs to be sorted on the prio member. The vertex descriptors are 'just'
 unsigned long numbers, so we can't store those in the queue as then it would be
 sorted on the vertex 'number' in stead of the priority.
 */
struct vertex_property_t {
  // TODO: make this a reference, it only works now 'by value' because vertex_t
  // is an unsigned long.
  vertex_t v;

  float prio = 0.f;
  float rank = 0.f;
  std::string label = "";

#if !XY_PAGERANK_USE_XYPRIOQUEUE
  typename fibonacci_heap<prio_q_element_t<vertex_t>,
                          boost::heap::compare<vertex_compare_t>>::handle_type
      handle;
#endif
};

#if !XY_PAGERANK_USE_XYPRIOQUEUE
// defines the priority queue type
typedef fibonacci_heap<prio_q_element_t<vertex_t>,
                       boost::heap::compare<vertex_compare_t>> prio_q_t;
#endif

/*! Personalised page rank function.
 \brief This function calculates and stores the personalise page ranks in each
 vertex based on a preselected 'target'.

 \tparam graph_t
 \tparam vertex_t
 \param graph
 \param probability
 \param tolerance
 \param target
 */
template <typename G, typename V>
auto XYPageRank(G &graph, float probability, float tolerance, V target,
                std::map<vertex_t, vertex_property_t> &vertexProperties,
                bool useEdgeWeights = false) -> int {
  typedef prio_q_element_t<V> pq_element_t;

// create the priority queue. A custom compare function is passed in as a
// lambda. This lambda gets the priority properties from the graph (from its
// capture block). Because of this it is now possible to refactor the graph to
// support dynamic properties instead of the struct based implementation that
// is currently still used.

#if XY_PAGERANK_USE_XYPRIOQUEUE
  XYPriorityQueue<prio_q_element_t<V>> PrioQ(
      [](const prio_q_element_t<V> &n1, const prio_q_element_t<V> &n2) {
        return n1.prio < n2.prio;
      });
#else
  prio_q_t PrioQ(
      [&graph](const prio_q_element_t<V> &n1, const prio_q_element_t<V> &n2) {
        return n1.prio < n2.prio;
      });
#endif

  // 𝑠[𝑣] = 𝑝[𝑣] = α
  // target := 𝑣, prio := 𝑝, rank := 𝑠, probability := α
  vertexProperties[target].prio = probability;
  pq_element_t TargetPrioQEl = {target, probability};
  vertexProperties[target].rank = probability;

  // 𝑞 = Max Priority Queue on 𝑉 ordered by key 𝑝
  // PrioQ := 𝑞
  vertexProperties[target].v = target;

#if XY_PAGERANK_USE_XYPRIOQUEUE
  PrioQ.push(TargetPrioQEl);
#else
  typename prio_q_t::handle_type handle = PrioQ.push(TargetPrioQEl);
  vertexProperties[target].handle = handle;
#endif

  // 𝑤 = 𝑞.popMaxElement()
  pq_element_t TopPrioEl = PrioQ.top();
  PrioQ.pop();

  auto ItemCounter = 0;
  auto MaxQItems = 0ul;

  // while 𝑞.maxPriority() > α ∙ 𝜖 do
  // tolerance := 𝜖
  while (TopPrioEl.prio > probability * tolerance) {
    // define iterators for in and out edges.
    typename graph_traits<G>::in_edge_iterator InEdgeIt, InEdgeEndIt;
    typename graph_traits<G>::out_edge_iterator OutEdgeIt, OutEdgeEndIt;

    // for 𝑢 in 𝑤.inNeighbors() do
    // InEdgeIt := 𝑢

    for (tie(InEdgeIt, InEdgeEndIt) = in_edges(TopPrioEl.element, graph);
         InEdgeIt != InEdgeEndIt; ++InEdgeIt) {
      auto DeltaS = 0.f;
      auto InNeighbour = source(*InEdgeIt, graph);

      /* if (useEdgeWeights) {
        auto CachedOutDegree = graph[*InEdgeIt].wOutDegree;
        auto WeightedOutDegree = 0.f;

        // ∑𝑤 weight[𝑢][𝑤]
        // the results are static for the graph, so we cache the results in the
        // edge properties

        if (CachedOutDegree >= 0.f)  // value was already cached
          WeightedOutDegree = CachedOutDegree;
        else  //!< value was not yet in the cache, so calculate and cache
          for (tie(OutEdgeIt, OutEdgeEndIt) = out_edges(InNeighbour, graph);
               OutEdgeIt != OutEdgeEndIt; ++OutEdgeIt)
            graph[*InEdgeIt].wOutDegree = WeightedOutDegree +=
                graph[*OutEdgeIt].weight;

        // ∆𝑠 = (1 - α) ∙ 𝑝[𝑤]weight[𝑢][𝑤] / 𝑢.weightedOutDegree
        // DeltaS := ∆𝑠

        DeltaS = (1 - probability) *
                 (TopPrioEl.prio * graph[*InEdgeIt].weight) / WeightedOutDegree;
      } else { */
      // ∆𝑠 = (1 - α) ∙ 𝑝[𝑤] / 𝑢.outDegree
      DeltaS =
          (1 - probability) * TopPrioEl.prio / out_degree(InNeighbour, graph);
      //}
      // we implement a sparse priority queue that contains only priorities > 0,
      // we keep the prio from the previous iteration
      auto OldPrio = vertexProperties[InNeighbour].prio;

      // 𝑠[𝑢] = 𝑠[𝑢] + ∆𝑠
      vertexProperties[InNeighbour].rank += DeltaS;

      // 𝑞.increasePriority(𝑢, 𝑝[𝑣] + ∆𝑠)
      vertexProperties[InNeighbour].prio += DeltaS;
      pq_element_t InNeighbourPrioQEl = {InNeighbour,
                                         vertexProperties[InNeighbour].prio};

      if (OldPrio == 0.f) {  // the vertex 𝑢 is not in the priority queue
        //(sparse queue), so we push into the queue

        vertexProperties[InNeighbour].v = InNeighbour;

#if XY_PAGERANK_USE_XYPRIOQUEUE
        PrioQ.push(InNeighbourPrioQEl);
#else
        typename prio_q_t::handle_type handle = PrioQ.push(InNeighbourPrioQEl);

        vertexProperties[InNeighbour].handle = handle;
#endif

      } else {  // the vertex 𝑢 was already in the priority queu, so we have
        // to 'increase' fibonacci_heap fixup interface
        if (!PrioQ.empty())

#if XY_PAGERANK_USE_XYPRIOQUEUE
          PrioQ.update(InNeighbourPrioQEl);
#else
          PrioQ.increase(vertexProperties[InNeighbour].handle,
                         InNeighbourPrioQEl);
#endif
        else
          std::cout << "update asked for empty queue" << std::endl;
      }
    }

    // 𝑝[𝑤] = 0
    vertexProperties[TopPrioEl.element].prio = 0.f;

    ItemCounter++;

    // update MaxQItems if size of the current queue is higher (used for
    // statistics).
    if (PrioQ.size() > MaxQItems) MaxQItems = PrioQ.size();

    // 𝑤 = 𝑞.popMaxElement()
    if (!PrioQ.empty()) {
      TopPrioEl = PrioQ.top();
      PrioQ.pop();
    } else
      break;
  }

// some basic temporary statistics output
#if XY_PAGERANK_USE_XYPRIOQUEUE
  std::cout << "Using XY Prio Queue\n";
#endif
  std::cout << "processed " << ItemCounter << "hops..." << std::endl;
  std::cout << "max amount of items on queue was: " << MaxQItems << std::endl;
  return ItemCounter;
}

// experimental graph cleaning function
template <typename G, typename V>
void cleanPageRankedGraph(G &graph, V target, float probability,
                          float tolerance,
                          std::map<V, int> &processedVertexes) {
  typename graph_traits<G>::in_edge_iterator InEdgeIt, InEdgeEndIt;

  for (tie(InEdgeIt, InEdgeEndIt) = in_edges(target, graph);
       InEdgeIt != InEdgeEndIt; ++InEdgeIt) {
    G InNeighbour = source(*InEdgeIt, graph);
    if (graph[InNeighbour].rank > 0.0f) {
      if (!processedVertexes[InNeighbour]) {
        processedVertexes[InNeighbour] = 1;
        cleanPageRankedGraph(graph, InNeighbour, probability, tolerance,
                             processedVertexes);
        graph[InNeighbour].rank = 0.0f;
        graph[InNeighbour].prio = 0.0f;
      }
    }
  }
  graph[target].rank = 0.0f;
  graph[target].prio = 0.0f;
}

}  // namespace XYGraphEngine

#endif /* defined(__PageRank__pPageRank__) */
