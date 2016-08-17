//
//  xy_lmdb_graph.h
//  PageRank
//
//  Created by Erik van der Tier on 27/02/15.
//
//

#ifndef XYGRAPHENGINE_XY_LMDB_GRAPH_H_
#define XYGRAPHENGINE_XY_LMDB_GRAPH_H_

#include "discovery_engine/cpp_lmdb/lmdb_database.h"
#include "discovery_engine/cpp_lmdb/lmdb_env.h"
#include "discovery_engine/cpp_lmdb/lmdb_cursor.h"

#include "boost/optional.hpp"
#include <vector>
#include <functional>

using namespace LMDB;

namespace XYGraphEngine {

#define GET_EDGE_VALUE(Buffer, index) \
  *(uint64_t *)(                      \
      ((DBVal *)((char *)(Buffer.buffer()) + sizeof(uint64_t) * (index))))

#define GET_EDGE_PTR(Buffer, index) \
  (uint64_t *)(                     \
      ((DBVal *)((char *)(Buffer.buffer()) + sizeof(uint64_t) * (index))))

template <typename ET>
struct edge {
  uint64_t vertexID1;
  uint64_t vertexID2;
  ET properties;
};

struct edge_id_t {
  uint64_t vertexID1;
  uint64_t vertexID2;
  size_t size() const { return sizeof(edge_id_t); };
  void *buffer() const { return (void *)this; };
};

template <typename VT>
struct vertex {
  uint64_t id;
  VT properties;
};

template <typename VT>
auto vertexCmp(const MDB_val *vertex1, const MDB_val *vertex2) -> int {
  uint64_t v1 = *static_cast<uint64_t *>(vertex1->mv_data);
  uint64_t v2 = *static_cast<uint64_t *>(vertex2->mv_data);
  if (v1 < v2)
    return -1;
  else if (v1 > v2)
    return 1;
  else
    return 0;
}

template <typename ET>
auto edgeCmp(const MDB_val *vertex1, const MDB_val *vertex2) -> int {
  edge_id_t *v1 = static_cast<edge_id_t *>(vertex1->mv_data);
  edge_id_t *v2 = static_cast<edge_id_t *>(vertex2->mv_data);
  if (v1->vertexID1 < v2->vertexID1)
    return -1;
  else if (v1->vertexID1 > v2->vertexID1)
    return 1;
  else {
    if (v1->vertexID2 < v2->vertexID2)
      return -1;
    else if (v1->vertexID2 > v2->vertexID2)
      return 1;
  }
  return 0;
}

const uint64_t kOutEdgeMask = 0x8000000000000000;
const uint16_t kMaxBatchItemsPerTransaction = 10000;

template <typename VT>
using vertex_processor_t =
    std::function<void(ErrorPtr &e, const uint64_t *vertexID1,
                       const DBVal *properties, LMDB::Transaction &txn)>;

template <typename ET>
using edge_processor_t = std::function<void(
    ErrorPtr &e, const uint64_t *vertexID1, const uint64_t *vertexID2,
    const DBVal *properties, LMDB::Transaction &txn)>;

// TODO: whereever we return a optional value the error should be reset on
// MDB_NOTFOUND
template <typename VT, typename ET>
class lmdb_graph {
 public:
  lmdb_graph(ErrorPtr &e, std::string dbPath, uint size, bool readOnly = false);
  lmdb_graph(ErrorPtr &e, LMDBEnv &env);
  virtual ~lmdb_graph();
  auto getEnv() -> LMDBEnv &;
  auto beginTransaction(ErrorPtr &e) const -> LMDB::Transaction;
  auto beginTransaction(ErrorPtr &e, unsigned flags) const -> LMDB::Transaction;
  auto beginTransaction(ErrorPtr &e, LMDB::Transaction &txn) const
      -> LMDB::Transaction;
  auto beginTransaction(ErrorPtr &e, unsigned flags,
                        LMDB::Transaction &txn) const -> LMDB::Transaction;
  void commitTransaction(ErrorPtr &e, LMDB::Transaction &txn) const;

  void addVertex(ErrorPtr &e, uint64_t vertexID, VT properties);
  void addVertex(ErrorPtr &e, uint64_t vertexID, VT properties,
                 LMDB::Transaction &txn);

  auto getVertex(ErrorPtr &e, uint64_t vertexID) const -> boost::optional<VT>;
  auto getVertex(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn) const
      -> boost::optional<VT>;

  void updateVertex(ErrorPtr &e, uint64_t id, VT properties);
  void removeVertex(ErrorPtr &e, uint64_t id);
  void removeVertex(ErrorPtr &e, uint64_t id, LMDB::Transaction &txn);

  void forEachVertex(ErrorPtr &e, vertex_processor_t<VT> vertexProcessor);
  void forEachVertex(ErrorPtr &e, LMDB::Transaction &txn,
                     vertex_processor_t<VT> vertexProcessor);

  auto getDegree(ErrorPtr &e, uint64_t vertexID) const -> uint64_t;
  auto getDegree(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn) const
      -> uint64_t;

  auto getInDegree(ErrorPtr &e, uint64_t vertexID) const -> uint64_t;
  auto getInDegree(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn) const
      -> uint64_t;

  auto getOutDegree(ErrorPtr &e, uint64_t vertexID) const -> uint64_t;
  auto getOutDegree(ErrorPtr &e, uint64_t vertexID,
                    LMDB::Transaction &txn) const -> uint64_t;

  auto getEdges(ErrorPtr &e, uint64_t vertexID) -> std::vector<edge<ET>>;
  auto getEdges(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn)
      -> std::vector<edge<ET>>;

  auto getInEdges(ErrorPtr &e, uint64_t vertexID) -> std::vector<edge<ET>>;
  auto getInEdges(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn)
      -> std::vector<edge<ET>>;

  auto getOutEdges(ErrorPtr &e, uint64_t vertexID) -> std::vector<edge<ET>>;
  auto getOutEdges(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn)
      -> std::vector<edge<ET>>;

  void forEachEdge(ErrorPtr &e, uint64_t vertexID,
                   edge_processor_t<ET> edgeProcessor);
  void forEachEdge(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn,
                   edge_processor_t<ET> edgeProcessor);

  void forEachInEdge(ErrorPtr &e, uint64_t vertexID,
                     edge_processor_t<ET> edgeProcessor);
  void forEachInEdge(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn,
                     edge_processor_t<ET> edgeProcessor);

  void forEachOutEdge(ErrorPtr &e, uint64_t vertexID,
                      edge_processor_t<ET> edgeProcessor);
  void forEachOutEdge(ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn,
                      edge_processor_t<ET> edgeProcessor);

  void addEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
               ET properties);
  void addEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
               ET properties, LMDB::Transaction &txn);
  void addEdgeBulk(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
                   ET properties, LMDB::Transaction &txn);
  void AjdListUpdate(ErrorPtr &e, uint64_t vertexID1,
                     std::vector<uint64_t> inEdges,
                     std::vector<uint64_t> OutEdges, LMDB::Transaction &txn);
  void addEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2);
  auto getEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2) const
      -> boost::optional<ET>;
  auto getEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
               LMDB::Transaction &txn) const -> boost::optional<ET>;
  auto getAnyEdges(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2)
      -> std::vector<edge<ET>>;
  auto getAnyEdges(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
                   LMDB::Transaction &txn) -> std::vector<edge<ET>>;

  auto getEdgePtr(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
                  LMDB::Transaction &txn) const -> ET *;
  void updateEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
                  ET properties);
  void removeEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2);
  void removeEdge(ErrorPtr &e, uint64_t vertexID1, uint64_t vertexID2,
                  LMDB::Transaction &txn);

  void clear(ErrorPtr &e);
  void remove(ErrorPtr &e);

 private:
  LMDBEnv StoreEnv_;
  LMDB::DatabasePtr<DBVal, DBVal> VertexDB_;
  LMDB::DatabasePtr<DBVal, DBVal> EdgeDB_;
  LMDB::DatabasePtr<DBVal, DBVal> AdjacencyListDB_;
};

template <typename VT, typename ET>
lmdb_graph<VT, ET>::lmdb_graph(ErrorPtr &e, std::string path, uint size,
                               bool readOnly)
    : StoreEnv_(e, path, 4, size) {
  ON_ERROR_RETURN(e);
  VertexDB_ =
      StoreEnv_.openDatabase<DBVal, DBVal>(e, "VertexDB", MDB_INTEGERKEY);
  EdgeDB_ = StoreEnv_.openDatabase<DBVal, DBVal>(e, "EdgeDB", 0, edgeCmp<ET>);
  AdjacencyListDB_ = StoreEnv_.openDatabase<DBVal, DBVal>(
      e, "AdjancencyListDB",
      MDB_INTEGERKEY | MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERDUP);
}

template <typename VT, typename ET>
lmdb_graph<VT, ET>::lmdb_graph(ErrorPtr &e, LMDBEnv &env)
    : StoreEnv_(env) {
  ON_ERROR_RETURN(e);
  VertexDB_ =
      StoreEnv_.openDatabase<DBVal, DBVal>(e, "VertexDB", MDB_INTEGERKEY);
  EdgeDB_ = StoreEnv_.openDatabase<DBVal, DBVal>(e, "EdgeDB", 0, edgeCmp<ET>);
  AdjacencyListDB_ = StoreEnv_.openDatabase<DBVal, DBVal>(
      e, "AdjancencyListDB",
      MDB_INTEGERKEY | MDB_DUPSORT | MDB_DUPFIXED | MDB_INTEGERDUP);
}

template <typename VT, typename ET>
lmdb_graph<VT, ET>::~lmdb_graph() {}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::beginTransaction(ErrorPtr &e) const
    -> LMDB::Transaction {
  return StoreEnv_.beginTransaction(e);
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::beginTransaction(ErrorPtr &e, unsigned flags) const
    -> LMDB::Transaction {
  return StoreEnv_.beginTransaction(e, flags);
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::beginTransaction(ErrorPtr &e,
                                          LMDB::Transaction &txn) const
    -> LMDB::Transaction {
  return StoreEnv_.beginTransaction(e, txn);
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::beginTransaction(ErrorPtr &e, unsigned flags,
                                          LMDB::Transaction &txn) const
    -> LMDB::Transaction {
  return StoreEnv_.beginTransaction(e, flags, txn);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::commitTransaction(ErrorPtr &e,
                                           LMDB::Transaction &txn) const {
  StoreEnv_.commitTransaction(e, txn);
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getEnv() -> LMDBEnv & {
  return StoreEnv_;
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::addVertex(ErrorPtr &e, uint64_t vertexID,
                                   VT properties) {
  ASSERT_ERROR_RESET(e);
  VertexDB_->add(e, vertexID, properties, true);
  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::addVertex(ErrorPtr &e, uint64_t vertexID,
                                   VT properties, LMDB::Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  auto ID = DBVal(vertexID);
  VertexDB_->add(e, ID, properties, txn, true);
  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getVertex(ErrorPtr &e, const uint64_t vertexID) const
    -> boost::optional<VT> {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN_NONE(e);
  auto out = VertexDB_->get(e, vertexID, txn);
  if (out == nullptr) return boost::none;
  ON_ERROR_RETURN_NONE(e);
  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_NONE(e);
  return boost::optional<VT>(dbValAs<VT>(*out));
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getVertex(ErrorPtr &e, const uint64_t vertexID,
                                   LMDB::Transaction &txn) const
    -> boost::optional<VT> {
  ASSERT_ERROR_RESET(e);
  auto out = VertexDB_->get(e, vertexID, txn);
  if (out == nullptr) return boost::none;
  ON_ERROR_RETURN_NONE(e);
  return boost::optional<VT>(dbValAs<VT>(*out));
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachVertex(ErrorPtr &e, LMDB::Transaction &txn,
                                       vertex_processor_t<VT> vertexProcessor) {
  ASSERT_ERROR_RESET(e);
  auto VertexCursor = VertexDB_->getCursor(e, txn);
  ON_ERROR_RETURN(e);

  auto Vertex = VertexCursor->getFirst(e);
  ON_ERROR_RETURN(e);

  while (!Vertex.is_initialized()) {
    vertexProcessor(e, static_cast<uint64_t *>((Vertex->key).buffer()),
                    static_cast<VT *>((Vertex->value).buffer()), txn);
    ON_ERROR_RETURN(e);
    Vertex = VertexCursor->getNext(e);
    ON_ERROR_RETURN(e);
  }
  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachVertex(ErrorPtr &e,
                                       vertex_processor_t<VT> vertexProcessor) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN(e);

  {
    auto VertexCursor = VertexDB_->getCursor(e, txn);
    ON_ERROR_RETURN(e);

    auto Vertex = VertexCursor->getFirst(e);

    ON_ERROR_RETURN(e);
    uint16_t IterCount = 0;
    while (!Vertex.is_initialized()) {
      vertexProcessor(e, static_cast<uint64_t *>((Vertex->key).buffer()),
                      &(Vertex->value), txn);
      ON_ERROR_RETURN(e);
      Vertex = VertexCursor->getNext(e);
      ON_ERROR_RETURN(e);
      if (++IterCount == kMaxBatchItemsPerTransaction) {
        IterCount = 0;
        ERROR_RESET(e);
        StoreEnv_.resetTransaction(e, txn);
        ON_ERROR_RETURN(e);
      }
    }
    ON_ERROR_RETURN(e);
  }
  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getOutDegree(ErrorPtr &e, uint64_t vertexID) const
    -> uint64_t {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, 0);

  auto OutDegree = getOutDegree(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, 0);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, 0);

  return OutDegree;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getOutDegree(ErrorPtr &e, uint64_t vertexID,
                                      LMDB::Transaction &txn) const
    -> uint64_t {
  ASSERT_ERROR_RESET(e);
  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  auto Edge = EdgeCursor->getFirstDup(e, vertexID | kOutEdgeMask);
  ON_ERROR_RETURN_VAL(e, 0);
  auto OutDegree = EdgeCursor->getCount(e);
  ON_ERROR_RETURN_VAL(e, 0);
  if (OutDegree)
    return *OutDegree;
  else
    return 0;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getDegree(ErrorPtr &e, uint64_t vertexID) const
    -> uint64_t {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, 0);

  auto Degree = getDegree(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, 0);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, 0);

  return Degree;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getDegree(ErrorPtr &e, uint64_t vertexID,
                                   LMDB::Transaction &txn) const -> uint64_t {
  ASSERT_ERROR_RESET(e);
  auto InDegree = getInDegree(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, 0);
  auto OutDegree = getOutDegree(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, 0);
  return InDegree + OutDegree;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getInDegree(ErrorPtr &e, uint64_t vertexID) const
    -> uint64_t {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, 0);

  auto InDegree = getInDegree(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, 0);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, 0);

  return InDegree;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getInDegree(ErrorPtr &e, uint64_t vertexID,
                                     LMDB::Transaction &txn) const -> uint64_t {
  ASSERT_ERROR_RESET(e);
  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  auto Edge = EdgeCursor->getFirstDup(e, vertexID);
  ON_ERROR_RETURN_VAL(e, 0);
  auto InDegree = EdgeCursor->getCount(e);
  ON_ERROR_RETURN_VAL(e, 0);
  if (InDegree.is_initialized())
    return *InDegree;
  else
    return 0;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getEdges(ErrorPtr &e, uint64_t vertexID)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, std::vector<edge<ET>>());

  auto Edges = getEdges(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, Edges);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, Edges);

  return Edges;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getEdges(ErrorPtr &e, uint64_t vertexID,
                                  LMDB::Transaction &txn)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  std::vector<edge<ET>> result;
  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  ON_ERROR_RETURN_VAL(e, result);

  auto Edge = EdgeCursor->getFirstDup(e, vertexID);
  if (!Edge.is_initialized()) return result;

  auto NumValues = *EdgeCursor->getCount(e);
  ON_ERROR_RETURN_VAL(e, result);
  if (NumValues == 1) {  // static_cast<VT*>((*Vertex->value).buffer())
    auto InEdgeProps = getEdge(
        e, *static_cast<uint64_t *>((Edge->value).buffer()), vertexID, txn);
    if (!InEdgeProps.is_initialized()) return result;

    result.push_back({*static_cast<uint64_t *>((Edge->value).buffer()),
                      vertexID, *InEdgeProps});
  } else {
    // DBVal *BulkValues;
    auto BulkValues = EdgeCursor->getFirstMultiple(e);
    ON_ERROR_RETURN_VAL(e, result);
    while (BulkValues.is_initialized()) {
      NumValues = BulkValues->value.size() / sizeof(uint64_t);
      for (int i = 0; i < NumValues; ++i) {
        uint64_t Value = GET_EDGE_VALUE(BulkValues->value, i);
        auto InEdgeProps = getEdge(e, Value, vertexID, txn);

        if (!InEdgeProps.is_initialized()) return result;

        result.push_back({Value, vertexID, *InEdgeProps});
      }
      BulkValues = EdgeCursor->getNextMultiple(e);
      ON_ERROR_RETURN_VAL(e, result);
    }
  }
  ERROR_RESET(e);

  Edge = EdgeCursor->getFirstDup(e, vertexID | kOutEdgeMask);
  if (!Edge.is_initialized()) return result;

  NumValues = *EdgeCursor->getCount(e);
  ON_ERROR_RETURN_VAL(e, result);
  if (NumValues == 1) {
    auto OutEdgeProps = getEdge(
        e, vertexID, *static_cast<uint64_t *>((Edge->value).buffer()), txn);

    if (!OutEdgeProps.is_initialized()) return result;

    result.push_back({vertexID,
                      *static_cast<uint64_t *>((Edge->value).buffer()),
                      *OutEdgeProps});
  } else {
    auto BulkValues = EdgeCursor->getFirstMultiple(e);
    ON_ERROR_RETURN_VAL(e, result);

    // process any pages associated with the current VertexKey.
    while (BulkValues.is_initialized()) {
      NumValues = BulkValues->value.size() / sizeof(uint64_t);
      // if this page starts with an 'outedge' process from index 0 upwards
      for (int i = 0; i < NumValues; ++i) {
        uint64_t Value = GET_EDGE_VALUE(BulkValues->value, i);
        // if bit 63 is set on the current value it is an out-edge
        auto OutEdgeProps = getEdge(e, vertexID, Value, txn);

        if (!OutEdgeProps.is_initialized()) return result;

        result.push_back({vertexID, Value, *OutEdgeProps});
        // OutEdgeCache_[vertexID].push_back({vertexID, Value,
        // *OutEdgeProps});
      }
      BulkValues = EdgeCursor->getNextMultiple(e);
      ON_ERROR_RETURN_VAL(e, result);
    }
  }

  return result;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getInEdges(ErrorPtr &e, uint64_t vertexID)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, std::vector<edge<ET>>());

  auto InEdges = getInEdges(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, InEdges);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, InEdges);
  return InEdges;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getInEdges(ErrorPtr &e, uint64_t vertexID,
                                    LMDB::Transaction &txn)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  std::vector<edge<ET>> result;

  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  ON_ERROR_RETURN_VAL(e, result);

  auto Edge = EdgeCursor->getFirstDup(e, vertexID);
  if (!Edge.is_initialized()) return result;

  auto NumValues = *EdgeCursor->getCount(e);
  ON_ERROR_RETURN_VAL(e, result);
  if (NumValues == 1) {
    auto InEdgeProps = getEdge(
        e, *static_cast<uint64_t *>((Edge->value).buffer()), vertexID, txn);
    if (!InEdgeProps.is_initialized()) return result;
    result.push_back({*static_cast<uint64_t *>((Edge->value).buffer()),
                      vertexID, *InEdgeProps});
  } else {
    auto BulkValues = EdgeCursor->getFirstMultiple(e);
    ON_ERROR_RETURN_VAL(e, result);
    while (BulkValues.is_initialized()) {
      NumValues = BulkValues->value.size() / sizeof(uint64_t);
      for (int i = 0; i < NumValues; ++i) {
        uint64_t Value = GET_EDGE_VALUE(BulkValues->value, i);
        auto InEdgeProps = getEdge(e, Value, vertexID, txn);
        if (!InEdgeProps.is_initialized()) return result;
        result.push_back({Value, vertexID, *InEdgeProps});
      }
      BulkValues = EdgeCursor->getNextMultiple(e);
      ON_ERROR_RETURN_VAL(e, result);
    }
  }

  return result;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getOutEdges(ErrorPtr &e, uint64_t vertexID)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, std::vector<edge<ET>>());

  auto OutEdges = getOutEdges(e, vertexID, txn);
  ON_ERROR_RETURN_VAL(e, OutEdges);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, OutEdges);
  return OutEdges;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getOutEdges(ErrorPtr &e, uint64_t vertexID,
                                     LMDB::Transaction &txn)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  std::vector<edge<ET>> result;

  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  ON_ERROR_RETURN_VAL(e, result);

  auto Edge = EdgeCursor->getFirstDup(e, vertexID | kOutEdgeMask);
  if (!Edge.is_initialized()) return result;

  auto NumValues = *EdgeCursor->getCount(e);
  ON_ERROR_RETURN_VAL(e, result);
  if (NumValues == 1) {
    auto OutEdgeProps = getEdge(
        e, vertexID, *static_cast<uint64_t *>((Edge->value).buffer()), txn);
    if (!OutEdgeProps.is_initialized()) return result;
    result.push_back({vertexID,
                      *static_cast<uint64_t *>((Edge->value).buffer()),
                      *OutEdgeProps});
  } else {
    auto BulkValues = EdgeCursor->getFirstMultiple(e);
    ON_ERROR_RETURN_VAL(e, result);

    // process any pages associated with the current VertexKey.
    while (BulkValues.is_initialized()) {
      NumValues = BulkValues->value.size() / sizeof(uint64_t);

      // if this page starts with an 'outedge' process from index 0 upwards
      for (int i = 0; i < NumValues; ++i) {
        uint64_t Value = GET_EDGE_VALUE(BulkValues->value, i);
        // if bit 63 is set on the current value it is an out-edge
        auto OutEdgeProps = getEdge(e, vertexID, Value, txn);
        if (!OutEdgeProps.is_initialized()) return result;
        result.push_back({vertexID, Value, *OutEdgeProps});
        // OutEdgeCache_[vertexID].push_back({vertexID, Value,
        // *OutEdgeProps});
      }
      BulkValues = EdgeCursor->getNextMultiple(e);
      ON_ERROR_RETURN_VAL(e, result);
    }
  }
  return result;
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachEdge(ErrorPtr &e, uint64_t vertexID,
                                     edge_processor_t<ET> edgeProcessor) {}
template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachEdge(
    ErrorPtr &e, uint64_t vertexID, LMDB::Transaction &txn,
    std::function<void(ErrorPtr &e, const uint64_t *vertexID1,
                       const uint64_t *vertexID2, const DBVal *properties,
                       LMDB::Transaction &txn)> edgeProcessor) {
  ASSERT_ERROR_RESET(e);
  forEachInEdge(e, vertexID, txn, edgeProcessor);
  ON_ERROR_RETURN(e);
  forEachOutEdge(e, vertexID, txn, edgeProcessor);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachInEdge(ErrorPtr &e, uint64_t vertexID,
                                       edge_processor_t<ET> edgeProcessor) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN(e);

  forEachInEdge(e, vertexID, txn, edgeProcessor);

  ON_ERROR_RETURN(e);
  StoreEnv_.commitTransaction(e, txn);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachInEdge(ErrorPtr &e, uint64_t vertexID,
                                       LMDB::Transaction &txn,
                                       edge_processor_t<ET> edgeProcessor) {
  ASSERT_ERROR_RESET(e);
  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  ON_ERROR_RETURN(e);

  auto Edge = EdgeCursor->getFirstDup(e, vertexID);
  if (!Edge.is_initialized()) return;

  auto NumValues = EdgeCursor->getCount(e);
  ON_ERROR_RETURN(e);
  if (*NumValues == 1) {
    const ET *InEdgeProps = getEdgePtr(
        e, *static_cast<uint64_t *>((Edge->value).buffer()), vertexID, txn);
    ON_ERROR_RETURN(e);
    auto edgeProps = InEdgeProps == nullptr ? DBVal() : DBVal(*InEdgeProps);
    edgeProcessor(e, static_cast<uint64_t *>((Edge->value).buffer()), &vertexID,
                  &edgeProps, txn);
    ON_ERROR_RETURN(e);

  } else {
    auto BulkValues = EdgeCursor->getFirstMultiple(e);
    ON_ERROR_RETURN(e);
    while (BulkValues.is_initialized()) {
      NumValues = BulkValues->value.size() / sizeof(uint64_t);

      for (int i = 0; i < *NumValues; i++) {
        uint64_t *Value = GET_EDGE_PTR(BulkValues->value, i);

        const ET *InEdgeProps = getEdgePtr(e, *Value, vertexID, txn);
        ON_ERROR_RETURN(e);
        auto edgeProps = InEdgeProps == nullptr ? DBVal() : DBVal(*InEdgeProps);
        edgeProcessor(e, Value, &vertexID, &edgeProps, txn);
        ON_ERROR_RETURN(e);
      }
      BulkValues = EdgeCursor->getNextMultiple(e);
      ON_ERROR_RETURN(e);
    }
  }
  if (e.code() == MDB_NOTFOUND) ERROR_RESET(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachOutEdge(ErrorPtr &e, uint64_t vertexID,
                                        edge_processor_t<ET> edgeProcessor) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN(e);

  forEachOutEdge(e, vertexID, txn, edgeProcessor);
  ON_ERROR_RETURN(e);
  StoreEnv_.commitTransaction(e, txn);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::forEachOutEdge(ErrorPtr &e, uint64_t vertexID,
                                        LMDB::Transaction &txn,
                                        edge_processor_t<ET> edgeProcessor) {
  ASSERT_ERROR_RESET(e);
  auto EdgeCursor = AdjacencyListDB_->getCursor(e, txn);
  ON_ERROR_RETURN(e);

  auto Edge = EdgeCursor->getFirstDup(e, vertexID | kOutEdgeMask);
  if (!Edge.is_initialized()) return;

  auto NumValues = EdgeCursor->getCount(e);
  ON_ERROR_RETURN(e);
  if (*NumValues == 1) {
    const ET *OutEdgeProps = getEdgePtr(
        e, vertexID, *static_cast<uint64_t *>((Edge->value).buffer()), txn);
    ON_ERROR_RETURN(e);
    auto edgeProps = OutEdgeProps == nullptr ? DBVal() : DBVal(*OutEdgeProps);
    edgeProcessor(e, &vertexID, static_cast<uint64_t *>((Edge->value).buffer()),
                  &edgeProps, txn);
    ON_ERROR_RETURN(e);

  } else {
    auto BulkValues = EdgeCursor->getFirstMultiple(e);
    ON_ERROR_RETURN(e);
    while (BulkValues.is_initialized()) {
      NumValues = BulkValues->value.size() / sizeof(uint64_t);

      for (int i = 0; i < *NumValues; ++i) {
        uint64_t *Value = GET_EDGE_PTR(BulkValues->value, i);
        const ET *OutEdgeProps = getEdgePtr(e, vertexID, *Value, txn);
        ON_ERROR_RETURN(e);
        auto edgeProps =
            OutEdgeProps == nullptr ? DBVal() : DBVal(*OutEdgeProps);
        edgeProcessor(e, &vertexID, Value, &edgeProps, txn);
        ON_ERROR_RETURN(e);
      }
      BulkValues = EdgeCursor->getNextMultiple(e);
      ON_ERROR_RETURN(e);
    }
  }
  if (e.code() == MDB_NOTFOUND) ERROR_RESET(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::updateVertex(ErrorPtr &e, uint64_t vertexID,
                                      VT properties) {}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::removeVertex(ErrorPtr &e, uint64_t vertexID) {
  // TODO: add removal of edges
  ASSERT_ERROR_RESET(e);
  VertexDB_->del(e, vertexID);
  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::removeVertex(ErrorPtr &e, uint64_t vertexID,
                                      LMDB::Transaction &txn) {
  // TODO: add removal of edges
  ASSERT_ERROR_RESET(e);
  VertexDB_->del(e, vertexID, txn);
  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::addEdge(ErrorPtr &e, uint64_t vertexID1,
                                 const uint64_t vertexID2, ET properties) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN(e);

  addEdge(e, vertexID1, vertexID2, properties, txn);

  StoreEnv_.commitTransaction(e, txn);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::addEdge(ErrorPtr &e, uint64_t vertexID1,
                                 uint64_t vertexID2, ET properties,
                                 LMDB::Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  edge_id_t EdgeID = {vertexID1, vertexID2};

  EdgeDB_->add(e, EdgeID, properties, txn, true);
  if (e.code() != MDB_KEYEXIST) ON_ERROR_RETURN(e);
  ERROR_RESET(e);
  AdjacencyListDB_->add(e, vertexID1 | kOutEdgeMask, vertexID2, txn, true);
  if (e.code() != MDB_KEYEXIST) ON_ERROR_RETURN(e);
  ERROR_RESET(e);
  AdjacencyListDB_->add(e, vertexID2, vertexID1, txn, true);
  if (e.code() != MDB_KEYEXIST) ON_ERROR_RETURN(e);
  ERROR_RESET(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::addEdgeBulk(ErrorPtr &e, uint64_t vertexID1,
                                     uint64_t vertexID2, ET properties,
                                     LMDB::Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  edge_id_t EdgeID = {vertexID1, vertexID2};
  DBVal dbVal = DBVal(sizeof(edge_id_t), (void *)&EdgeID);
  EdgeDB_->add(e, dbVal, properties, txn, true);
  if (e.code() != MDB_KEYEXIST) ON_ERROR_RETURN(e);
  ERROR_RESET(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::AjdListUpdate(ErrorPtr &e, uint64_t vertexID,
                                       std::vector<uint64_t> inEdges,
                                       std::vector<uint64_t> outEdges,
                                       LMDB::Transaction &txn) {
  ASSERT_ERROR_RESET(e);

  for (auto InEdgeID : inEdges) {
    AdjacencyListDB_->add(e, vertexID, InEdgeID, txn, false);
    ON_ERROR_RETURN(e);
  }
  for (auto OutEdgeID : outEdges) {
    AdjacencyListDB_->add(e, vertexID | kOutEdgeMask, OutEdgeID, txn, false);
    ON_ERROR_RETURN(e);
  }
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::addEdge(ErrorPtr &e, uint64_t vertexID1,
                                 uint64_t vertexID2) {}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getEdge(ErrorPtr &e, uint64_t vertexID1,
                                 uint64_t vertexID2) const
    -> boost::optional<ET> {
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN_NONE(e);
  edge_id_t EdgeID = {vertexID1, vertexID2};
  DBVal dbVal = {sizeof(edge_id_t), (void *)&EdgeID};

  auto out = EdgeDB_->get(e, dbVal, txn);
  ON_ERROR_RETURN_NONE(e);
  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_NONE(e);
  if (out == nullptr) return boost::none;
  return *static_cast<ET *>(out->buffer());
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getEdge(ErrorPtr &e, uint64_t vertexID1,
                                 uint64_t vertexID2,
                                 LMDB::Transaction &txn) const
    -> boost::optional<ET> {
  edge_id_t EdgeID = {vertexID1, vertexID2};
  DBVal dbVal = {sizeof(edge_id_t), (void *)&EdgeID};
  auto out = EdgeDB_->get(e, dbVal, txn);
  ON_ERROR_RETURN_NONE(e);
  if (out == nullptr) return boost::none;
  return *static_cast<ET *>(out->buffer());
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getAnyEdges(ErrorPtr &e, uint64_t vertexID1,
                                     uint64_t vertexID2)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e, MDB_RDONLY);
  ON_ERROR_RETURN_VAL(e, std::vector<edge<ET>>());

  auto Edges = getAnyEdges(e, vertexID1, vertexID2, txn);
  ON_ERROR_RETURN_VAL(e, Edges);

  StoreEnv_.commitTransaction(e, txn);
  ON_ERROR_RETURN_VAL(e, Edges);
  return Edges;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getAnyEdges(ErrorPtr &e, uint64_t vertexID1,
                                     uint64_t vertexID2, LMDB::Transaction &txn)
    -> std::vector<edge<ET>> {
  ASSERT_ERROR_RESET(e);
  std::vector<edge<ET>> result;

  auto Edge = getEdge(e, vertexID1, vertexID2, txn);
  if (Edge.is_initialized()) result.push_back({vertexID1, vertexID2, *Edge});
  ON_ERROR_RETURN_VAL(e, result);
  Edge = getEdge(e, vertexID2, vertexID1, txn);
  if (Edge.is_initialized()) result.push_back({vertexID2, vertexID1, *Edge});
  return result;
}

template <typename VT, typename ET>
auto lmdb_graph<VT, ET>::getEdgePtr(ErrorPtr &e, uint64_t vertexID1,
                                    uint64_t vertexID2,
                                    LMDB::Transaction &txn) const -> ET * {
  ASSERT_ERROR_RESET(e);
  edge_id_t EdgeID = {vertexID1, vertexID2};
  DBVal dbVal = {sizeof(edge_id_t), (void *)&EdgeID};
  auto out = EdgeDB_->get(e, dbVal, txn);
  ON_ERROR_RETURN_VAL(e, nullptr);
  if (out == nullptr) return nullptr;
  return static_cast<ET *>(out->buffer());
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::updateEdge(ErrorPtr &e, uint64_t vertexID1,
                                    uint64_t vertexID2, ET properties) {}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::removeEdge(ErrorPtr &e, uint64_t vertexID1,
                                    uint64_t vertexID2) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN(e);

  removeEdge(e, vertexID1, vertexID2, txn);
  ON_ERROR_RETURN(e);

  StoreEnv_.commitTransaction(e, txn);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::removeEdge(ErrorPtr &e, uint64_t vertexID1,
                                    uint64_t vertexID2,
                                    LMDB::Transaction &txn) {
  ASSERT_ERROR_RESET(e);
  edge_id_t EdgeID = {vertexID1, vertexID2};
  DBVal dbVal = {sizeof(edge_id_t), (void *)&EdgeID};

  EdgeDB_->del(e, dbVal, txn);

  ERROR_RESET(e);
  AdjacencyListDB_->del(e, vertexID1 | kOutEdgeMask, vertexID2, txn);
  if (e.code() != MDB_NOTFOUND) ON_ERROR_RETURN(e);
  ERROR_RESET(e);
  AdjacencyListDB_->del(e, vertexID2, vertexID1, txn);
  if (e.code() != MDB_NOTFOUND) ON_ERROR_RETURN(e);
  ERROR_RESET(e);

  ON_ERROR_RETURN(e);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::clear(ErrorPtr &e) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN(e);

  EdgeDB_->clear(e, txn);
  ON_ERROR_RETURN(e);
  VertexDB_->clear(e, txn);
  ON_ERROR_RETURN(e);
  AdjacencyListDB_->clear(e, txn);
  ON_ERROR_RETURN(e);

  StoreEnv_.commitTransaction(e, txn);
}

template <typename VT, typename ET>
void lmdb_graph<VT, ET>::remove(ErrorPtr &e) {
  ASSERT_ERROR_RESET(e);
  auto txn = StoreEnv_.beginTransaction(e);
  ON_ERROR_RETURN(e);

  StoreEnv_.dropDatabase(e, EdgeDB_, txn);
  ON_ERROR_RETURN(e);
  StoreEnv_.dropDatabase(e, VertexDB_, txn);
  ON_ERROR_RETURN(e);
  StoreEnv_.dropDatabase(e, AdjacencyListDB_, txn);
  ON_ERROR_RETURN(e);

  StoreEnv_.commitTransaction(e, txn);

  // TODO: remove database files as well
}

}  // namespace XYGraphEngine

#endif  // XYGRAPHENGINE_XY_LMDB_GRAPH_H_
