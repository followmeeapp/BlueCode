/*! \file XYPageRank.h
 \brief Implementation of a templated function that calculates the 'personalised
 page rank' for a specified target vertex.

 Created by Erik van der Tier on 10/01/15.
 Copyright (c) 2015 Xy Group Llc. All rights reserved.
*/

#ifndef XYGRAPHENGINE_XY_PAGERANK_H_
#define XYGRAPHENGINE_XY_PAGERANK_H_

#include "xy_pagerank_config.h"
#include "xy_lmdb_graph.h"
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

struct vertex_property_t;

struct edge_property_t {
  float weight = 0.0;
  float wOutDegree = -1.0;
};

// Graph type with custom vertex and edge properties: vProp_t and eProp_t
// typedef adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
//                       vertex_property_t, edge_property_t> graph_t;
// Type of the vertices and edges
// typedef typename graph_traits<graph_t>::vertex_descriptor vertex_t;
// typedef typename graph_traits<graph_t>::edge_descriptor edge_t;

// Type of the priority queue elements. T
template <typename T>
struct prio_q_element_t {
  T element;
  float prio;
  bool operator==(const prio_q_element_t<T> &rhv) {
    return element == rhv.element;
  }
};

typedef function<bool(const prio_q_element_t<uint64_t> &n1,
                      const prio_q_element_t<uint64_t> &n2)> vertex_compare_t;

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
  uint64_t v;

  float prio = 0.f;
  float rank = 0.f;

#if !XY_PAGERANK_USE_XYPRIOQUEUE
  typename fibonacci_heap<prio_q_element_t<uint64_t>,
                          boost::heap::compare<vertex_compare_t>>::handle_type
      handle;
#endif
};

#if !XY_PAGERANK_USE_XYPRIOQUEUE
// defines the priority queue type
typedef fibonacci_heap<prio_q_element_t<uint64_t>,
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
                std::map<uint64_t, vertex_property_t> &vertexProperties,
                bool useEdgeWeights, LMDB::Transaction &txn) -> int {
  typedef prio_q_element_t<V> pq_element_t;

#if XY_PAGERANK_USE_XYPRIOQUEUE
  XYPriorityQueue<prio_q_element_t<V>> PrioQ(
      [](const prio_q_element_t<V> &n1, const prio_q_element_t<V> &n2) {
        return n1.prio < n2.prio;
      });
#else
  prio_q_t PrioQ(
      [](const prio_q_element_t<V> &n1, const prio_q_element_t<V> &n2) {
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

  ErrorPtr e;

  // while 𝑞.maxPriority() > α ∙ 𝜖 do
  // tolerance := 𝜖
  while (TopPrioEl.prio > probability * tolerance) {
    // define iterators for in and out edges.

    // for 𝑢 in 𝑤.inNeighbors() do
    // InEdgeIt := 𝑢
    ERROR_RESET(e);

    graph.forEachInEdge(
        e, TopPrioEl.element, txn,
        [&graph, &TopPrioEl, &vertexProperties, &PrioQ, &probability,
         &useEdgeWeights](ErrorPtr &e, const uint64_t *vertexID1,
                          const uint64_t *vertexID2, const DBVal *properties,
                          LMDB::Transaction &txn) {
          auto DeltaS = 0.f;
          auto InNeighbour = *vertexID1;

          if (useEdgeWeights) {
            auto WeightedOutDegree = 0.f;

            //∑𝑤 weight[𝑢][𝑤]

            graph.forEachOutEdge(
                e, InNeighbour, txn,
                [&graph, &WeightedOutDegree](
                    ErrorPtr &e, const uint64_t *vertexID1,
                    const uint64_t *vertexID2, const DBVal *properties,
                    LMDB::Transaction &txn) {
                  WeightedOutDegree += *(float *)properties->buffer();
                });

            // ∆𝑠 = (1 - α) ∙ 𝑝[𝑤]weight[𝑢][𝑤] / 𝑢.weightedOutDegree
            // DeltaS := ∆𝑠

            DeltaS = (1 - probability) *
                     (TopPrioEl.prio * *(float *)properties->buffer()) /
                     WeightedOutDegree;
          } else {
            // ∆𝑠 = (1 - α) ∙ 𝑝[𝑤] / 𝑢.outDegree

            auto OutDegree = graph.getOutDegree(e, InNeighbour, txn);
            DeltaS = (1 - probability) * TopPrioEl.prio / OutDegree;
          }
          // we implement a sparse priority queue that contains only priorities
          // > 0,
          // we keep the prio from the previous iteration
          auto OldPrio = vertexProperties[InNeighbour].prio;

          // 𝑠[𝑢] = 𝑠[𝑢] + ∆𝑠
          vertexProperties[InNeighbour].rank += DeltaS;

          // 𝑞.increasePriority(𝑢, 𝑝[𝑣] + ∆𝑠)
          vertexProperties[InNeighbour].prio += DeltaS;
          pq_element_t InNeighbourPrioQEl = {
              InNeighbour, vertexProperties[InNeighbour].prio};

          if (OldPrio == 0.f) {  // the vertex 𝑢 is not in the priority queue
                                 //(sparse queue), so we push into the queue

            vertexProperties[InNeighbour].v = InNeighbour;

#if XY_PAGERANK_USE_XYPRIOQUEUE
            PrioQ.push(InNeighbourPrioQEl);
#else
            typename prio_q_t::handle_type handle =
                PrioQ.push(InNeighbourPrioQEl);

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
              std::cout << "update asked for empty queue\n";
          }
        });
    if (e) {
      if (e.get()->code != MDB_NOTFOUND) {
        std::cout << e.get()->message();
      } else {
        ERROR_RESET(e);
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
  ERROR_RESET(e);

// some basic temporary statistics output
#if XY_PAGERANK_USE_XYPRIOQUEUE
// std::cout << "Using XY Prio Queue\n";
#endif
  // std::cout << "max amount of items on queue was: " << MaxQItems << "\n";
  return ItemCounter;
}

}  // namespace XYGraphEngine

#endif /* defined(__PageRank__pPageRank__) */
